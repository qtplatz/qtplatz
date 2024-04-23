// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "settingsdialog.h"

#include "ioptionspage.h"
#include "../coreplugintr.h"
#include "../icore.h"
#include "../iwizardfactory.h"

#include <utils/algorithm.h>
#include <utils/fancylineedit.h>
#include <utils/guiutils.h>
#include <utils/hostosinfo.h>
#include <utils/icon.h>
#include <utils/qtcassert.h>
#include <utils/stylehelper.h>

#include <QAbstractSpinBox>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListView>
#include <QPointer>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QSpacerItem>
#include <QStackedLayout>
#include <QStyle>
#include <QStyledItemDelegate>

#include <extensionsystem/pluginmanager.h>

const int kInitialWidth = 750;
const int kInitialHeight = 450;
const int kMaxMinimumWidth = 250;
const int kMaxMinimumHeight = 250;

static const char pageKeyC[] = "General/LastPreferencePage";
static const char sortKeyC[] = "General/SortCategories";
const int categoryIconSize = 24;

using namespace Utils;

namespace Core {
namespace Internal {

bool optionsPageLessThan(const IOptionsPage *p1, const IOptionsPage *p2)
{
    if (p1->category() != p2->category())
        return p1->category().alphabeticallyBefore(p2->category());
    return p1->id().alphabeticallyBefore(p2->id());
}

static inline QList<IOptionsPage*> sortedOptionsPages()
{
    QList<IOptionsPage*> rc = IOptionsPage::allOptionsPages();
    std::stable_sort(rc.begin(), rc.end(), optionsPageLessThan);
    return rc;
}

// ----------- Category model

class Category
{
public:
    bool findPageById(const Id id, int *pageIndex) const
    {
        *pageIndex = Utils::indexOf(pages, Utils::equal(&IOptionsPage::id, id));
        return *pageIndex != -1;
    }

    Id id;
    int index = -1;
    QString displayName;
    QIcon icon;
    QList<IOptionsPage *> pages;
    QList<IOptionsPageProvider *> providers;
    bool providerPagesCreated = false;
    QTabWidget *tabWidget = nullptr;
};

class CategoryModel : public QAbstractListModel
{
public:
    CategoryModel();
    ~CategoryModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setPages(const QList<IOptionsPage*> &pages,
                  const QList<IOptionsPageProvider *> &providers);
    void ensurePages(Category *category);
    const QList<Category*> &categories() const { return m_categories; }

private:
    Category *findCategoryById(Id id);

    QList<Category*> m_categories;
    QSet<Id> m_pageIds;
    QIcon m_emptyIcon;
};

CategoryModel::CategoryModel()
{
    QPixmap empty(categoryIconSize, categoryIconSize);
    empty.fill(Qt::transparent);
    m_emptyIcon = QIcon(empty);
}

CategoryModel::~CategoryModel()
{
    qDeleteAll(m_categories);
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_categories.size();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return m_categories.at(index.row())->displayName;
    case Qt::DecorationRole: {
            QIcon icon = m_categories.at(index.row())->icon;
            if (icon.isNull())
                icon = m_emptyIcon;
            return icon;
        }
    }

    return QVariant();
}

void CategoryModel::setPages(const QList<IOptionsPage*> &pages,
                             const QList<IOptionsPageProvider *> &providers)
{
    beginResetModel();

    // Clear any previous categories
    qDeleteAll(m_categories);
    m_categories.clear();
    m_pageIds.clear();

    // Put the pages in categories
    for (IOptionsPage *page : pages) {
        QTC_ASSERT(!m_pageIds.contains(page->id()),
                   qWarning("duplicate options page id '%s'", qPrintable(page->id().toString())));
        m_pageIds.insert(page->id());
        const Id categoryId = page->category();
        Category *category = findCategoryById(categoryId);
        if (!category) {
            category = new Category;
            category->id = categoryId;
            category->tabWidget = nullptr;
            category->index = -1;
            m_categories.append(category);
        }
        if (category->displayName.isEmpty())
            category->displayName = page->displayCategory();
        if (category->icon.isNull() && !page->categoryIconPath().isEmpty()) {
            Icon icon({{page->categoryIconPath(), Theme::PanelTextColorDark}}, Icon::Tint);
            category->icon = icon.icon();
        }
        category->pages.append(page);
    }

    for (IOptionsPageProvider *provider : providers) {
        const Id categoryId = provider->category();
        Category *category = findCategoryById(categoryId);
        if (!category) {
            category = new Category;
            category->id = categoryId;
            category->tabWidget = nullptr;
            category->index = -1;
            m_categories.append(category);
        }
        if (category->displayName.isEmpty())
            category->displayName = provider->displayCategory();
        if (category->icon.isNull()) {
            Icon icon({{provider->categoryIconPath(), Theme::PanelTextColorDark}}, Icon::Tint);
            category->icon = icon.icon();
        }
        category->providers.append(provider);
    }

    Utils::sort(m_categories, [](const Category *c1, const Category *c2) {
       return c1->id.alphabeticallyBefore(c2->id);
    });
    endResetModel();
}

void CategoryModel::ensurePages(Category *category)
{
    if (!category->providerPagesCreated) {
        QList<IOptionsPage *> createdPages;
        for (const IOptionsPageProvider *provider : std::as_const(category->providers))
            createdPages += provider->pages();

        // check for duplicate ids
        for (const IOptionsPage *page : std::as_const(createdPages)) {
            QTC_ASSERT(!m_pageIds.contains(page->id()),
                       qWarning("duplicate options page id '%s'", qPrintable(page->id().toString())));
        }

        category->pages += createdPages;
        category->providerPagesCreated = true;
        std::stable_sort(category->pages.begin(), category->pages.end(), optionsPageLessThan);
    }
}

Category *CategoryModel::findCategoryById(Id id)
{
    for (int i = 0; i < m_categories.size(); ++i) {
        Category *category = m_categories.at(i);
        if (category->id == id)
            return category;
    }

    return nullptr;
}

// ----------- Category filter model

/**
 * A filter model that returns true for each category node that has pages that
 * match the search string.
 */
class CategoryFilterModel : public QSortFilterProxyModel
{
public:
    CategoryFilterModel() = default;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

const char SETTING_HIDE_OPTION_CATEGORIES[] = "HideOptionCategories";

static bool categoryVisible(const Id &id)
{
    static QStringList list
        = Core::ICore::settings()->value(SETTING_HIDE_OPTION_CATEGORIES).toStringList();

    if (anyOf(list, [id](const QString &str) { return id.toString().contains(str); }))
        return false;

    return true;
}

bool CategoryFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const CategoryModel *cm = static_cast<CategoryModel *>(sourceModel());
    const Category *category = cm->categories().at(sourceRow);

    if (!categoryVisible(category->id))
        return false;

    // Regular contents check, then check page-filter.
    if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
        return true;

    const QRegularExpression regex = filterRegularExpression();

    for (const IOptionsPage *page : category->pages) {
        if (page->displayCategory().contains(regex) || page->displayName().contains(regex)
            || page->matches(regex))
            return true;
    }

    if (!category->providerPagesCreated) {
        for (const IOptionsPageProvider *provider : category->providers) {
            if (provider->matches(regex))
                return true;
        }
    }

    return false;
}

// ----------- Category list view

class CategoryListViewDelegate : public QStyledItemDelegate
{
public:
    explicit CategoryListViewDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 32));
        return size;
    }
};

/**
 * Special version of a QListView that has the width of the first column as
 * minimum size.
 */
class CategoryListView : public QListView
{
public:
    CategoryListView()
    {
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        setItemDelegate(new CategoryListViewDelegate(this));
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    QSize sizeHint() const final
    {
        int width = sizeHintForColumn(0) + frameWidth() * 2 + 5;
        width += verticalScrollBar()->sizeHint().width();
        return QSize(width, 100);
    }

    // QListView installs a event filter on its scrollbars
    bool eventFilter(QObject *obj, QEvent *event) final
    {
        if (obj == verticalScrollBar()
                && (event->type() == QEvent::Show || event->type() == QEvent::Hide))
            updateGeometry();
        return QListView::eventFilter(obj, event);
    }
};

// ----------- SmartScrollArea

template <typename T>
void setWheelScrollingWithoutFocusBlockedForChildren(QWidget *widget)
{
    const auto children = widget->findChildren<T>();
    for (auto child : children)
        setWheelScrollingWithoutFocusBlocked(child);
}

class SmartScrollArea : public QScrollArea
{
public:
    explicit SmartScrollArea(QWidget *parent, IOptionsPage *page)
        : QScrollArea(parent), m_page(page)
    {
        setFrameStyle(QFrame::NoFrame | QFrame::Plain);
        viewport()->setAutoFillBackground(false);
        setWidgetResizable(true);
    }

private:
    void showEvent(QShowEvent *event) final
    {
        if (!widget()) {
            if (QWidget *inner = m_page->widget()) {
                setWheelScrollingWithoutFocusBlockedForChildren<QComboBox *>(inner);
                setWheelScrollingWithoutFocusBlockedForChildren<QAbstractSpinBox *>(inner);
                setWidget(inner);
                inner->setAutoFillBackground(false);
            } else {
                QTC_CHECK(false);
            }
        }

        QScrollArea::showEvent(event);
    }

    void resizeEvent(QResizeEvent *event) final
    {
        QWidget *inner = widget();
        if (inner) {
            int fw = frameWidth() * 2;
            QSize innerSize = event->size() - QSize(fw, fw);
            QSize innerSizeHint = inner->minimumSizeHint();

            if (innerSizeHint.height() > innerSize.height()) { // Widget wants to be bigger than available space
                innerSize.setWidth(innerSize.width() - scrollBarWidth());
                innerSize.setHeight(innerSizeHint.height());
            }
            inner->resize(innerSize);
        }
        QScrollArea::resizeEvent(event);
    }

    QSize minimumSizeHint() const final
    {
        QWidget *inner = widget();
        if (inner) {
            int fw = frameWidth() * 2;

            QSize minSize = inner->minimumSizeHint();
            minSize += QSize(fw, fw);
            minSize += QSize(scrollBarWidth(), 0);
            minSize.setWidth(qMin(minSize.width(), kMaxMinimumWidth));
            minSize.setHeight(qMin(minSize.height(), kMaxMinimumHeight));
            return minSize;
        }
        return QSize(0, 0);
    }

    bool event(QEvent *event) final
    {
        if (event->type() == QEvent::LayoutRequest)
            updateGeometry();
        return QScrollArea::event(event);
    }

    int scrollBarWidth() const
    {
        auto that = const_cast<SmartScrollArea *>(this);
        QWidgetList list = that->scrollBarWidgets(Qt::AlignRight);
        if (list.isEmpty())
            return 0;
        return list.first()->sizeHint().width();
    }

    IOptionsPage *m_page = nullptr;
};

// ----------- SettingsDialog

class SettingsDialog : public QDialog
{
public:
    explicit SettingsDialog(QWidget *parent);

    void showPage(Id pageId);
    bool execDialog();

private:
    // Make sure the settings dialog starts up as small as possible.
    QSize sizeHint() const final { return minimumSize(); }

    void done(int) final;
    void accept() final;
    void reject() final;

    void apply();
    void currentChanged(const QModelIndex &current);
    void currentTabChanged(int);
    void filter(const QString &text);

    void createGui();
    void showCategory(int index);
    static void updateEnabledTabs(Category *category, const QString &searchText);
    void ensureCategoryWidget(Category *category);
    void disconnectTabWidgets();

    const QList<IOptionsPage *> m_pages;

    QSet<IOptionsPage *> m_visitedPages;
    CategoryFilterModel m_proxyModel;
    CategoryModel m_model;
    Id m_currentCategory;
    Id m_currentPage;
    QStackedLayout *m_stackedLayout;
    Utils::FancyLineEdit *m_filterLineEdit;
    QCheckBox *m_sortCheckBox;
    QListView *m_categoryList;
    QLabel *m_headerLabel;
    bool m_running = false;
    bool m_applied = false;
    bool m_finished = false;
};

static QPointer<SettingsDialog> m_instance = nullptr;

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_pages(sortedOptionsPages())
    , m_stackedLayout(new QStackedLayout)
    , m_filterLineEdit(new Utils::FancyLineEdit)
    , m_sortCheckBox(new QCheckBox(Tr::tr("Sort categories")))
    , m_categoryList(new CategoryListView)
    , m_headerLabel(new QLabel)
{
    m_filterLineEdit->setFiltering(true);

    createGui();
    setWindowTitle(Tr::tr("Preferences"));

    m_model.setPages(m_pages, IOptionsPageProvider::allOptionsPagesProviders());

    m_proxyModel.setSortLocaleAware(true);
    m_proxyModel.setSourceModel(&m_model);
    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_categoryList->setIconSize(QSize(categoryIconSize, categoryIconSize));
    m_categoryList->setModel(&m_proxyModel);
    m_categoryList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_categoryList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(m_sortCheckBox, &QAbstractButton::toggled, this, [this](bool checked) {
        m_proxyModel.sort(checked ? 0 : -1);
    });
    QtcSettings *settings = ICore::settings();
    m_sortCheckBox->setChecked(settings->value(sortKeyC, false).toBool());

    connect(m_categoryList->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &SettingsDialog::currentChanged);

    // The order of the slot connection matters here, the filter slot
    // opens the matching page after the model has filtered.
    connect(m_filterLineEdit,
            &Utils::FancyLineEdit::filterChanged,
            &m_proxyModel,
            [this](const QString &filter) {
                m_proxyModel.setFilterRegularExpression(
                    QRegularExpression(QRegularExpression::escape(filter),
                                       QRegularExpression::CaseInsensitiveOption));
            });
    connect(m_filterLineEdit, &Utils::FancyLineEdit::filterChanged,
            this, &SettingsDialog::filter);
    m_categoryList->setFocus();
}

void SettingsDialog::showPage(const Id pageId)
{
    // handle the case of "show last page"
    Id initialPageId = pageId;
    if (!initialPageId.isValid()) {
        QtcSettings *settings = ICore::settings();
        initialPageId = Id::fromSetting(settings->value(pageKeyC));
    }

    int initialCategoryIndex = -1;
    int initialPageIndex = -1;

    const QList<Category *> &categories = m_model.categories();
    if (initialPageId.isValid()) {
        // First try categories without lazy items.
        for (int i = 0; i < categories.size(); ++i) {
            Category *category = categories.at(i);
            if (category->providers.isEmpty()) {  // no providers
                if (category->findPageById(initialPageId, &initialPageIndex)) {
                    initialCategoryIndex = i;
                    break;
                }
            }
        }

        if (initialPageIndex == -1) {
            // On failure, expand the remaining items.
            for (int i = 0; i < categories.size(); ++i) {
                Category *category = categories.at(i);
                if (!category->providers.isEmpty()) { // has providers
                    ensureCategoryWidget(category);
                    if (category->findPageById(initialPageId, &initialPageIndex)) {
                        initialCategoryIndex = i;
                        break;
                    }
                }
            }
        }
    }

    if (initialPageId.isValid() && initialPageIndex == -1)
        return; // Unknown settings page, probably due to missing plugin.

    if (initialCategoryIndex != -1) {
        QModelIndex modelIndex = m_proxyModel.mapFromSource(m_model.index(initialCategoryIndex));
        if (!modelIndex.isValid()) { // filtered out, so clear filter first
            m_filterLineEdit->setText(QString());
            modelIndex = m_proxyModel.mapFromSource(m_model.index(initialCategoryIndex));
        }
        m_categoryList->setCurrentIndex(modelIndex);
        if (initialPageIndex != -1) {
            if (QTC_GUARD(categories.at(initialCategoryIndex)->tabWidget))
                categories.at(initialCategoryIndex)->tabWidget->setCurrentIndex(initialPageIndex);
        }
    }
}

void SettingsDialog::createGui()
{
    m_headerLabel->setFont(StyleHelper::uiFont(StyleHelper::UiElementH4));

    auto headerHLayout = new QHBoxLayout;
    const int leftMargin = QApplication::style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
    headerHLayout->addSpacerItem(new QSpacerItem(leftMargin, 0, QSizePolicy::Fixed, QSizePolicy::Ignored));
    headerHLayout->addWidget(m_headerLabel);

    m_stackedLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *emptyWidget = new QWidget(this);
    m_stackedLayout->addWidget(emptyWidget); // no category selected, for example when filtering

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Apply |
                                                       QDialogButtonBox::Cancel);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked,
            this, &SettingsDialog::apply);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

    auto mainGridLayout = new QGridLayout;
    mainGridLayout->addWidget(m_filterLineEdit, 0, 0, 1, 1);
    mainGridLayout->addLayout(headerHLayout,    0, 1, 1, 1);
    mainGridLayout->addWidget(m_categoryList,   1, 0, 1, 1);
    mainGridLayout->addWidget(m_sortCheckBox,   2, 0, 1, 1);
    mainGridLayout->addLayout(m_stackedLayout,  1, 1, 2, 1);
    mainGridLayout->addWidget(buttonBox,        3, 0, 1, 2);
    mainGridLayout->setColumnStretch(1, 4);
    setLayout(mainGridLayout);

    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    mainGridLayout->setSizeConstraint(QLayout::SetMinimumSize);
}

void SettingsDialog::showCategory(int index)
{
    Category *category = m_model.categories().at(index);
    ensureCategoryWidget(category);
    // Update current category and page
    m_currentCategory = category->id;
    const int currentTabIndex = category->tabWidget->currentIndex();
    if (currentTabIndex != -1) {
        IOptionsPage *page = category->pages.at(currentTabIndex);
        m_currentPage = page->id();
        m_visitedPages.insert(page);
    }

    m_stackedLayout->setCurrentIndex(category->index);
    m_headerLabel->setText(category->displayName);

    updateEnabledTabs(category, m_filterLineEdit->text());
}

void SettingsDialog::ensureCategoryWidget(Category *category)
{
    if (category->tabWidget)
        return;

    m_model.ensurePages(category);
    auto tabWidget = new QTabWidget;
    tabWidget->tabBar()->setObjectName("qc_settings_main_tabbar"); // easier lookup in Squish
    for (IOptionsPage *page : std::as_const(category->pages))
        tabWidget->addTab(new SmartScrollArea(this, page), page->displayName());

    connect(tabWidget, &QTabWidget::currentChanged,
            this, &SettingsDialog::currentTabChanged);

    category->tabWidget = tabWidget;
    category->index = m_stackedLayout->addWidget(tabWidget);
}

void SettingsDialog::disconnectTabWidgets()
{
    for (Category *category : m_model.categories()) {
        if (category->tabWidget)
            disconnect(category->tabWidget, &QTabWidget::currentChanged,
                       this, &SettingsDialog::currentTabChanged);
    }
}

void SettingsDialog::updateEnabledTabs(Category *category, const QString &searchText)
{
    int firstEnabledTab = -1;
    const QRegularExpression regex(QRegularExpression::escape(searchText),
                                   QRegularExpression::CaseInsensitiveOption);
    for (int i = 0; i < category->pages.size(); ++i) {
        const IOptionsPage *page = category->pages.at(i);
        const bool enabled = searchText.isEmpty() || page->category().toString().contains(regex)
                             || page->displayName().contains(regex) || page->matches(regex);
        category->tabWidget->setTabEnabled(i, enabled);
        if (enabled && firstEnabledTab < 0)
            firstEnabledTab = i;
    }
    if (!category->tabWidget->isTabEnabled(category->tabWidget->currentIndex())
            && firstEnabledTab != -1) {
        // QTabWidget is dumb, so this can happen
        category->tabWidget->setCurrentIndex(firstEnabledTab);
    }
}

void SettingsDialog::currentChanged(const QModelIndex &current)
{
    if (current.isValid()) {
        showCategory(m_proxyModel.mapToSource(current).row());
    } else {
        m_stackedLayout->setCurrentIndex(0);
        m_headerLabel->clear();
    }
}

void SettingsDialog::currentTabChanged(int index)
{
    if (index == -1)
        return;

    const QModelIndex modelIndex = m_proxyModel.mapToSource(m_categoryList->currentIndex());
    if (!modelIndex.isValid())
        return;

    // Remember the current tab and mark it as visited
    const Category *category = m_model.categories().at(modelIndex.row());
    IOptionsPage *page = category->pages.at(index);
    m_currentPage = page->id();
    m_visitedPages.insert(page);
}

void SettingsDialog::filter(const QString &text)
{
    // When there is no current index, select the first one when possible
    if (!m_categoryList->currentIndex().isValid() && m_model.rowCount() > 0)
        m_categoryList->setCurrentIndex(m_proxyModel.index(0, 0));

    const QModelIndex currentIndex = m_proxyModel.mapToSource(m_categoryList->currentIndex());
    if (!currentIndex.isValid())
        return;

    Category *category = m_model.categories().at(currentIndex.row());
    updateEnabledTabs(category, text);
}

void SettingsDialog::accept()
{
    if (m_finished)
        return;
    m_finished = true;
    disconnectTabWidgets();
    m_applied = true;
    for (IOptionsPage *page : std::as_const(m_visitedPages))
        page->apply();
    for (IOptionsPage *page : std::as_const(m_pages))
        page->finish();
    done(QDialog::Accepted);
}

void SettingsDialog::reject()
{
    if (m_finished)
        return;
    m_finished = true;
    disconnectTabWidgets();
    for (IOptionsPage *page : std::as_const(m_pages))
        page->finish();
    done(QDialog::Rejected);
}

void SettingsDialog::apply()
{
    for (IOptionsPage *page : std::as_const(m_visitedPages))
        page->apply();
    m_applied = true;
}

void SettingsDialog::done(int val)
{
    QtcSettings *settings = ICore::settings();
    settings->setValue(pageKeyC, m_currentPage.toSetting());
    settings->setValue(sortKeyC, m_sortCheckBox->isChecked());

    ICore::saveSettings(ICore::SettingsDialogDone); // save all settings

    QDialog::done(val);
}

bool SettingsDialog::execDialog()
{
    if (!m_running) {
        m_running = true;
        m_finished = false;
        static const char kPreferenceDialogSize[] = "Core/PreferenceDialogSize";
        const QSize initialSize(kInitialWidth, kInitialHeight);
        resize(ICore::settings()->value(kPreferenceDialogSize, initialSize).toSize());

        // We call open here as exec is no longer the preferred method of displaying
        // modal dialogs. The issue that triggered the change here was QTBUG-117814
        // (on macOS: Caps Lock indicator does not update)
        open();
        connect(this, &QDialog::finished, this, [this, initialSize] {
            m_running = false;
            m_instance = nullptr;
            ICore::settings()->setValueWithDefault(kPreferenceDialogSize, size(), initialSize);
            // make sure that the current "single" instance is deleted
            // we can't delete right away, since we still access the m_applied member
            deleteLater();
        });
    }

    // This function needs to be blocking, so we need to wait for the dialog to finish.
    // We cannot use QDialog::exec due to the issue described above at "open()".
    // Since execDialog can be called multiple times, we need to run potentially multiple
    // loops here, to have every invocation of execDialog() wait for the dialog to finish.
    while (m_running)
        QApplication::processEvents(QEventLoop::WaitForMoreEvents);

    return m_applied;
}

bool executeSettingsDialog(QWidget *parent, Id initialPage)
{
    if (!ExtensionSystem::PluginManager::isInitializationDone()) {
        QObject::connect(ExtensionSystem::PluginManager::instance(),
                         &ExtensionSystem::PluginManager::initializationDone,
                         parent,
                         [parent, initialPage]() { executeSettingsDialog(parent, initialPage); });
        return false;
    }

    // Make sure all wizards are there when the user might access the keyboard shortcuts:
    (void) IWizardFactory::allWizardFactories();

    if (!m_instance)
        m_instance = new SettingsDialog(parent);

    m_instance->showPage(initialPage);
    return m_instance->execDialog();
}

} // namespace Internal
} // namespace Core
