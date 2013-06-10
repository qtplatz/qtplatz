// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

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
