// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

pragma Singleton
import QtQuick 2.15
import QtQuickDesignerTheme 1.0

QtObject {
    id: values

    property real baseHeight: 29
    property real baseFont: 12
    property real mediumFont: 14
    property real bigFont: 16
    property real baseIconFont: 12
    property real bigIconFont: 26

    property real scaleFactor: 1.0

    property real height: Math.round(values.baseHeight * values.scaleFactor)
    property real baseFontSize: Math.round(values.baseFont * values.scaleFactor)
    property real myFontSize: values.baseFontSize // TODO: rename all refs to myFontSize -> baseFontSize then remove myFontSize
    property real mediumFontSize: Math.round(values.mediumFont * values.scaleFactor)
    property real bigFontSize: Math.round(values.bigFont * values.scaleFactor)
    property real baseIconFontSize: Math.round(values.baseIconFont * values.scaleFactor)
    property real myIconFontSize: values.baseIconFontSize;  // TODO: rename all refs to myIconFontSize -> baseIconFontSize then remove myIconFontSize
    property real bigIconFontSize: Math.round(values.bigIconFont * values.scaleFactor)

    property real squareComponentWidth: values.height
    property real smallRectWidth: values.height / 2 * 1.5

    property real inputWidth: values.height * 4

    property real sliderHeight: values.height / 2 * 1.5 // TODO:Have a look at -> sliderAreaHeight: Data.Values.height/2*1.5

    property real sliderControlSize: 12
    property real sliderControlSizeMulti: values.sliderControlSize * values.scaleFactor

    property int dragThreshold: 10 // px
    property real spinControlIconSize: 8
    property real spinControlIconSizeMulti: values.spinControlIconSize * values.scaleFactor

    property real sliderTrackHeight: values.height / 3
    property real sliderHandleHeight: values.sliderTrackHeight * 1.8
    property real sliderHandleWidth: values.sliderTrackHeight * 0.5
    property real sliderFontSize: Math.round(8 * values.scaleFactor)
    property real sliderPadding: Math.round(6 * values.scaleFactor)
    property real sliderMargin: Math.round(3 * values.scaleFactor)

    property real sliderPointerWidth: Math.round(7 * values.scaleFactor)
    property real sliderPointerHeight: Math.round(2 * values.scaleFactor)

    property real checkBoxSpacing: Math.round(6 * values.scaleFactor)

    property real radioButtonSpacing: values.checkBoxSpacing
    property real radioButtonWidth: values.height
    property real radioButtonHeight: values.height
    property real radioButtonIndicatorWidth: 14
    property real radioButtonIndicatorHeight: 14

    property real switchSpacing: values.checkBoxSpacing

    property real columnWidth: 225 + (175 * (values.scaleFactor * 2))

    property real marginTopBottom: 4
    property real border: 1
    property real borderHover: 3

    property real maxComboBoxPopupHeight: Math.round(300 * values.scaleFactor)
    property real maxTextAreaPopupHeight: Math.round(150 * values.scaleFactor)

    property real contextMenuLabelSpacing: Math.round(30 * values.scaleFactor)
    property real contextMenuHorizontalPadding: Math.round(6 * values.scaleFactor)

    property real inputHorizontalPadding: Math.round(6 * values.scaleFactor)
    property real typeLabelVerticalShift: Math.round(6 * values.scaleFactor)

    property real scrollBarThickness: 10
    property real scrollBarActivePadding: 1
    property real scrollBarInactivePadding: 2

    property real toolTipHeight: 25
    property int toolTipDelay: 1000

    // Controls hover animation params
    property int hoverDuration: 500
    property int hoverEasing: Easing.OutExpo

    // Layout sizes
    property real sectionColumnSpacing: 20 // distance between label and sliderControlSize
    property real sectionRowSpacing: 5
    property real sectionHeadGap: 15
    property real sectionHeadHeight: 21 // tab and section
    property real sectionHeadSpacerHeight: 10

    property real controlLabelWidth: 15
    property real controlLabelGap: 5

    property real controlGap: 5 // TODO different name
    property real twoControlColumnGap: values.controlLabelGap
                                       + values.controlLabelWidth
                                       + values.controlGap

    property real columnGap: 10

    property real iconAreaWidth: Math.round(21 * values.scaleFactor)

    property real linkControlWidth: values.iconAreaWidth
    property real linkControlHeight: values.height

    property real infinityControlWidth: values.iconAreaWidth
    property real infinityControlHeight: values.height

    property real transform3DSectionSpacing: 15

    // Control sizes

    property real defaultControlWidth: values.squareComponentWidth * 5
    property real defaultControlHeight: values.height

    property real actionIndicatorWidth: values.iconAreaWidth //StudioTheme.Values.squareComponentWidth
    property real actionIndicatorHeight: values.height

    property real spinBoxIndicatorWidth: values.smallRectWidth - 2 * values.border
    property real spinBoxIndicatorHeight: values.height / 2 - values.border

    property real sliderIndicatorWidth: values.squareComponentWidth
    property real sliderIndicatorHeight: values.height

    property real translationIndicatorWidth: values.squareComponentWidth
    property real translationIndicatorHeight: values.height

    property real checkIndicatorWidth: values.squareComponentWidth
    property real checkIndicatorHeight: values.height

    property real singleControlColumnWidth: 2 * values.twoControlColumnWidth
                                            + values.twoControlColumnGap
                                            + values.actionIndicatorWidth

    property real twoControlColumnWidthMin: 3 * values.height - 2 * values.border
    property real twoControlColumnWidthMax: 3 * values.twoControlColumnWidthMin
    property real twoControlColumnWidth: values.twoControlColumnWidthMin

    property real controlColumnWithoutControlsWidth: 2 * (values.actionIndicatorWidth
                                                          + values.twoControlColumnGap)
                                                    + values.linkControlWidth

    property real controlColumnWidth: values.controlColumnWithoutControlsWidth
                                      + 2 * values.twoControlColumnWidth

    property real controlColumnWidthMin: values.controlColumnWithoutControlsWidth
                                         + 2 * values.twoControlColumnWidthMin

    property real propertyLabelWidthMin: 80
    property real propertyLabelWidthMax: 120
    property real propertyLabelWidth: values.propertyLabelWidthMin

    property real sectionLeftPadding: 8
    property real sectionLayoutRightPadding: values.scrollBarThickness + 6

    property real columnFactor: values.propertyLabelWidthMin
                                / (values.propertyLabelWidthMin + values.controlColumnWidthMin)

    function responsiveResize(width) {
        var tmpWidth = width - values.sectionColumnSpacing
                       - values.sectionLeftPadding - values.sectionLayoutRightPadding
        var labelColumnWidth = Math.round(tmpWidth * values.columnFactor)
        labelColumnWidth = Math.max(Math.min(values.propertyLabelWidthMax, labelColumnWidth),
                                    values.propertyLabelWidthMin)

        var controlColumnWidth = tmpWidth - labelColumnWidth
        var controlWidth = Math.round((controlColumnWidth - values.controlColumnWithoutControlsWidth) * 0.5)
        controlWidth = Math.max(Math.min(values.twoControlColumnWidthMax, controlWidth),
                                values.twoControlColumnWidthMin)

        values.propertyLabelWidth = labelColumnWidth
        values.twoControlColumnWidth = controlWidth
    }

    // Color Editor Popup
    property real colorEditorPopupWidth: 4 * values.colorEditorPopupSpinBoxWidth
                                         + 3 * values.controlGap
                                         + 2 * values.colorEditorPopupPadding
    property real colorEditorPopupHeight: 800
    property real colorEditorPopupPadding: 10
    property real colorEditorPopupMargin: 20

    property real colorEditorPopupSpacing: 10
    property real colorEditorPopupLineHeight: 60

    property real hueSliderHeight: 20
    property real hueSliderHandleWidth: 10

    property real colorEditorPopupCmoboBoxWidth: 110
    property real colorEditorPopupSpinBoxWidth: 54

    // Toolbar
    property real toolbarHeight: 35
    property real toolbarSpacing: 8

    // Dialog
    property real dialogPadding: 12
    property real dialogButtonSpacing: 10
    property real dialogButtonPadding: 4

    // Theme Colors

    property bool isLightTheme: themeControlBackground.hsvValue > themeTextColor.hsvValue

    property string themePanelBackground: Theme.color(Theme.DSpanelBackground)

    property string themeGreenLight: Theme.color(Theme.DSgreenLight)
    property string themeAmberLight: Theme.color(Theme.DSamberLight)
    property string themeRedLight: Theme.color(Theme.DSredLight)

    property string themeInteraction: Theme.color(Theme.DSinteraction)
    property string themeError: Theme.color(Theme.DSerrorColor)
    property string themeWarning: Theme.color(Theme.DSwarningColor)
    property string themeDisabled: Theme.color(Theme.DSdisabledColor)

    property string themeInteractionHover: Theme.color(Theme.DSinteractionHover)

    property string themeAliasIconChecked: Theme.color(Theme.DSnavigatorAliasIconChecked)

    // Control colors
    property color themeControlBackground: Theme.color(Theme.DScontrolBackground)
    property string themeControlBackgroundInteraction: Theme.color(Theme.DScontrolBackgroundInteraction)
    property string themeControlBackgroundDisabled: Theme.color(Theme.DScontrolBackgroundDisabled)
    property string themeControlBackgroundGlobalHover: Theme.color(Theme.DScontrolBackgroundGlobalHover)
    property string themeControlBackgroundHover: Theme.color(Theme.DScontrolBackgroundHover)

    property string themeControlOutline: Theme.color(Theme.DScontrolOutline)
    property string themeControlOutlineInteraction: Theme.color(Theme.DScontrolOutlineInteraction)
    property string themeControlOutlineDisabled: Theme.color(Theme.DScontrolOutlineDisabled)

    // Panels & Panes
    property string themeBackgroundColorNormal: Theme.color(Theme.DSBackgroundColorNormal)
    property string themeBackgroundColorAlternate: Theme.color(Theme.DSBackgroundColorAlternate)

    // Text colors
    property color themeTextColor: Theme.color(Theme.DStextColor)
    property string themeTextColorDisabled: Theme.color(Theme.DStextColorDisabled)
    property string themeTextSelectionColor: Theme.color(Theme.DStextSelectionColor)
    property string themeTextSelectedTextColor: Theme.color(Theme.DStextSelectedTextColor)
    property string themeTextColorDisabledMCU: Theme.color(Theme.DStextColorDisabled)

    property string themePlaceholderTextColor: Theme.color(Theme.DSplaceholderTextColor)
    property string themePlaceholderTextColorInteraction: Theme.color(Theme.DSplaceholderTextColorInteraction)

    // Icon colors
    property string themeIconColor: Theme.color(Theme.DSiconColor)
    property string themeIconColorHover: Theme.color(Theme.DSiconColorHover)
    property string themeIconColorInteraction: Theme.color(Theme.DSiconColorInteraction)
    property string themeIconColorDisabled: Theme.color(Theme.DSiconColorDisabled)
    property string themeIconColorSelected: Theme.color(Theme.DSiconColorSelected)

    property string themeLinkIndicatorColor: Theme.color(Theme.DSlinkIndicatorColor)
    property string themeLinkIndicatorColorHover: Theme.color(Theme.DSlinkIndicatorColorHover)
    property string themeLinkIndicatorColorInteraction: Theme.color(Theme.DSlinkIndicatorColorInteraction)
    property string themeLinkIndicatorColorDisabled: Theme.color(Theme.DSlinkIndicatorColorDisabled)

    property string themeInfiniteLoopIndicatorColor: Theme.color(Theme.DSlinkIndicatorColor)
    property string themeInfiniteLoopIndicatorColorHover: Theme.color(Theme.DSlinkIndicatorColorHover)
    property string themeInfiniteLoopIndicatorColorInteraction: Theme.color(Theme.DSlinkIndicatorColorInteraction)

    // Popup background color (ComboBox, SpinBox, TextArea)
    property string themePopupBackground: Theme.color(Theme.DSpopupBackground)
    // GradientPopupDialog modal overly color
    property string themePopupOverlayColor: Theme.color(Theme.DSpopupOverlayColor)

    // ToolTip (UrlChooser)
    property string themeToolTipBackground: Theme.color(Theme.DStoolTipBackground)
    property string themeToolTipOutline: Theme.color(Theme.DStoolTipOutline)
    property string themeToolTipText: Theme.color(Theme.DStoolTipText)

    // Slider colors
    property string themeSliderActiveTrack: Theme.color(Theme.DSsliderActiveTrack)
    property string themeSliderActiveTrackHover: Theme.color(Theme.DSsliderActiveTrackHover)
    property string themeSliderActiveTrackFocus: Theme.color(Theme.DSsliderActiveTrackFocus)
    property string themeSliderInactiveTrack: Theme.color(Theme.DSsliderInactiveTrack)
    property string themeSliderInactiveTrackHover: Theme.color(Theme.DSsliderInactiveTrackHover)
    property string themeSliderInactiveTrackFocus: Theme.color(Theme.DSsliderInactiveTrackFocus)
    property string themeSliderHandle: Theme.color(Theme.DSsliderHandle)
    property string themeSliderHandleHover: Theme.color(Theme.DSsliderHandleHover)
    property string themeSliderHandleFocus: Theme.color(Theme.DSsliderHandleFocus)
    property string themeSliderHandleInteraction: Theme.color(Theme.DSsliderHandleInteraction)

    property string themeScrollBarTrack: Theme.color(Theme.DSscrollBarTrack)
    property string themeScrollBarHandle: Theme.color(Theme.DSscrollBarHandle)

    property string themeSectionHeadBackground: Theme.color(Theme.DSsectionHeadBackground)

    property string themeTabActiveBackground: Theme.color(Theme.DStabActiveBackground)
    property string themeTabActiveText: Theme.color(Theme.DStabActiveText)
    property string themeTabInactiveBackground: Theme.color(Theme.DStabInactiveBackground)
    property string themeTabInactiveText: Theme.color(Theme.DStabInactiveText)

    // State Editor
    property string themeStateSeparator: Theme.color(Theme.DSstateSeparatorColor)
    property string themeStateBackground: Theme.color(Theme.DSstateBackgroundColor)
    property string themeStatePreviewOutline: Theme.color(Theme.DSstatePreviewOutline)

    // State Editor *new*
    property color themeStatePanelBackground: Theme.color(Theme.DSstatePanelBackground)
    property color themeStateHighlight: Theme.color(Theme.DSstateHighlight)

    property string themeUnimportedModuleColor: Theme.color(Theme.DSUnimportedModuleColor)

    // Taken out of Constants.js
    property string themeChangedStateText: Theme.color(Theme.DSchangedStateText)

    // 3D
    property string theme3DAxisXColor: Theme.color(Theme.DS3DAxisXColor)
    property string theme3DAxisYColor: Theme.color(Theme.DS3DAxisYColor)
    property string theme3DAxisZColor: Theme.color(Theme.DS3DAxisZColor)

    property string themeActionBinding: Theme.color(Theme.DSactionBinding)
    property string themeActionAlias: Theme.color(Theme.DSactionAlias)
    property string themeActionKeyframe: Theme.color(Theme.DSactionKeyframe)
    property string themeActionJIT: Theme.color(Theme.DSactionJIT)

    property string themeListItemBackground: Theme.color(Theme.DSnavigatorItemBackground)
    property string themeListItemBackgroundHover: Theme.color(Theme.DSnavigatorItemBackgroundHover)
    property string themeListItemBackgroundPress: Theme.color(Theme.DSnavigatorItemBackgroundSelected)
    property string themeListItemText: Theme.color(Theme.DSnavigatorText)
    property string themeListItemTextHover: Theme.color(Theme.DSnavigatorTextHover)
    property string themeListItemTextPress: Theme.color(Theme.DSnavigatorTextSelected)

    // Welcome Page
    property string welcomeScreenBackground: Theme.color(Theme.DSwelcomeScreenBackground)
    property string themeSubPanelBackground: Theme.color(Theme.DSsubPanelBackground)
    property string themeThumbnailBackground: Theme.color(Theme.DSthumbnailBackground)
    property string themeThumbnailLabelBackground: Theme.color(Theme.DSthumbnailLabelBackground)

    // Dialog
    property color themeDialogBackground: values.themeThumbnailBackground
    property color themeDialogOutline: values.themeInteraction
}
