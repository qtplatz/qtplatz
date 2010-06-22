#ifndef ACQUIREACTIONS_H
#define ACQUIREACTIONS_H

#include <QtCore/QHash>
#include <utils/savedaction.h>

QT_BEGIN_NAMESPACE
class QActionGroup;
QT_END_NAMESPACE

#if 0
namespace Acquire {
  namespace internal {

    class AcquireSettings : public QObject {
      Q_OBJECT
    public:
      AcquireSettings(QObject *parent = 0);
      ~AcquireSettings();
      
      void insertItem(int code, Utils::SavedAction *item);
      Utils::SavedAction *item(int code) const;
      
      QString dump() const;
      
      static AcquireSettings *instance();
      
      public slots:
      void readSettings(QSettings *settings);
      void writeSettings(QSettings *settings) const;
      
    private:
      QHash<int, Utils::SavedAction *> m_items; 
    };
    
    
    ///////////////////////////////////////////////////////////
      
      enum AcquireActionCode	{
	// General
	SettingsDialog,
	AdjustColumnWidths,
	AlwaysAdjustColumnWidths,
	UseAlternatingRowColors,
	UseMessageBoxForSignals,
	AutoQuit,
	LockView,
	LogTimeStamps,
	OperateByInstruction,
	AutoDerefPointers,
	
	RecheckDebuggingHelpers,
	UseDebuggingHelpers,
	UseCustomDebuggingHelperLocation,
	CustomDebuggingHelperLocation,
	DebugDebuggingHelpers,
	
	UseCodeModel,
	
	UseToolTipsInMainEditor,
	UseToolTipsInLocalsView,
	UseToolTipsInBreakpointsView,
	UseToolTipsInStackView,
	UseAddressInBreakpointsView,
	UseAddressInStackView,
	
	// Gdb
	GdbLocation,
	GdbEnvironment,
	GdbScriptFile,
	ExecuteCommand,
	GdbWatchdogTimeout,
	
	// Stack
	MaximalStackDepth,
	ExpandStack,
	
	// Watchers & Locals
	WatchExpression,
	WatchExpressionInWindow,
	RemoveWatchExpression,
	WatchPoint,
	AssignValue,
	AssignType,
	
	// Source List
	ListSourceFiles,
	
	// Running
	SkipKnownFrames,
	EnableReverseDebugging,
	
	// Breakpoints
	SynchronizeBreakpoints,
	AllPluginBreakpoints,
	SelectedPluginBreakpoints,
	NoPluginBreakpoints,
	SelectedPluginBreakpointsPattern,
	UsePreciseBreakpoints
      };

      // singleton access
      Utils::SavedAction *theAcquireAction(int code);
      
      // convenience
      bool theAcquireBoolSetting(int code);
      QString theAcquireStringSetting(int code);
      
      struct AcquireManagerActions
      {
	QAction *continueAction;
	QAction *stopAction;
	QAction *stepAction;
	QAction *stepOutAction;
	QAction *runToLineAction;
	QAction *runToFunctionAction;
	QAction *jumpToLineAction;
	QAction *nextAction;
	QAction *watchAction;
	QAction *breakAction;
	QAction *sepAction;
	QAction *reverseDirectionAction;
      };
      
  } // namespace Internal
} // namespace Acquire
#endif // 0

#endif // ACQUIREACTIONS_H
