//
//  CrashRecoveryHandler.cpp
//  interface/src
//
//  Created by David Rowe on 24 Aug 2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "CrashRecoveryHandler.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QLabel>
#include <PathUtils.h>
#include <QRadioButton>
#include <QCheckBox>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QtCore/QUrl>

#include "Application.h"
#include "Menu.h"



#include <RunningMarker.h>
#include <SettingHandle.h>
#include <SettingHelpers.h>
#include <SettingManager.h>
#include <DependencyManager.h>
#include <UserActivityLogger.h>
#include <BuildInfo.h>

bool CrashRecoveryHandler::checkForResetSettings(bool wasLikelyCrash, bool suppressPrompt) {
    Setting::Handle<bool> crashReportingAsked { "CrashReportingAsked", false };


    Settings settings;
    settings.beginGroup("Developer");
    QVariant displayCrashOptions = settings.value(MenuOption::DisplayCrashOptions);
    settings.endGroup();
    settings.beginGroup("Settings");
    QVariant askToResetSettingsOption = settings.value(MenuOption::AskToResetSettings);
    settings.endGroup();
    bool askToResetSettings = askToResetSettingsOption.isValid() && askToResetSettingsOption.toBool();

    // If option does not exist in Interface.ini so assume default behavior.
    bool displaySettingsResetOnCrash = !displayCrashOptions.isValid() || displayCrashOptions.toBool();
    bool userPrompted = false;

    if (suppressPrompt) {
        return wasLikelyCrash;
    }

    if (wasLikelyCrash || askToResetSettings) {
        if (displaySettingsResetOnCrash || askToResetSettings) {
            userPrompted = true;
            Action action = promptUserForAction(wasLikelyCrash);
            if (action != DO_NOTHING) {
                handleCrash(action);
            }
        }
    }

    if (!userPrompted) {
        // Both dialogs share a purpose -- if we already showed the full dialog, no need to show this one.
        if (BuildInfo::BUILD_TYPE != BuildInfo::BuildType::Stable) {
            // We didn't crash but are running a development build -- we'd like reports if possible.
            if (suggestCrashReporting()) {
                // If suggestCrashReporting returns false, we didn't ask the user.
                crashReportingAsked.set(true);
            }
        }

    }

    return wasLikelyCrash;
}


bool CrashRecoveryHandler::suggestCrashReporting() {
    QDialog crashDialog;

    crashDialog.setWindowTitle("Overte");

    QVBoxLayout* layout = new QVBoxLayout;

    QString explainText;
    auto &ual = UserActivityLogger::getInstance();



    switch(BuildInfo::BUILD_TYPE) {
        case BuildInfo::BuildType::Dev:
            explainText = "You're running a pre-release version. This is an official release, but the code\n"
                        "is not yet considered to be fully stable. We'd highly appreciate it if you enabled\n"
                        "crash reporting to help us test the upcoming release.";
            break;
        case BuildInfo::BuildType::PR:
            // TODO: It would be nice to have here the PR number, and who submitted it. This would require GHA support.
            explainText = "You're running a PR version. This is experimental code contributed by a third party\n"
                        "and not yet part of the official code. We'd highly appreciate it if you enabled\n"
                        "crash reporting to help us test this potential addition.";
            break;
        case BuildInfo::BuildType::Master:
            explainText = "You're running a pre-release version. This is an official release, but the code\n"
                        "is not yet considered to be fully stable. We'd highly appreciate it if you enabled\n"
                        "crash reporting to help us test the upcoming release.";
            break;
        case BuildInfo::BuildType::Stable:
            explainText = "You're running a stable version. This is an official release, and should perform\n"
                          "correctly. Nevertheless, we'd highly appreciate it if you enabled crash reporting\n"
                          "to help us catch any remaining problems.";
            break;
    }

    if (!ual.isCrashMonitorStarted()) {
        qWarning() << "Crash reporting not working, skipping suggestion to enable it.";
        return false;
    }

    QLabel* explainLabel = new QLabel(explainText);

    QLabel* crashReportLabel = new QLabel("Reports can only be seen by developers trusted by the Overte e.V.\n"
                                          "organization, and will only be used for improving the code.");

    QCheckBox* crashReportCheckbox = new QCheckBox("Enable automatic crash reporting");

    crashReportCheckbox->setChecked(ual.isCrashReportingEnabled());
    crashReportCheckbox->setEnabled(ual.isCrashMonitorStarted());

    layout->addWidget(explainLabel);
    layout->addSpacing(12);
    layout->addWidget(crashReportLabel);
    layout->addWidget(crashReportCheckbox);
    layout->addSpacing(12);
    layout->addStretch();

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(buttons);
    crashDialog.connect(buttons, SIGNAL(accepted()), SLOT(accept()));

    crashDialog.setLayout(layout);

    crashDialog.exec();

    ual.setCrashReportingEnabled(crashReportCheckbox->isChecked());

    return true;
}

CrashRecoveryHandler::Action CrashRecoveryHandler::promptUserForAction(bool showCrashMessage) {
    QDialog crashDialog;
    QLabel* label;
    auto &ual = UserActivityLogger::getInstance();

    if (showCrashMessage) {
        crashDialog.setWindowTitle("Interface Crashed Last Run");
        label = new QLabel("If you are having trouble starting would you like to reset your settings?");
    } else {
        crashDialog.setWindowTitle("Reset Settings");
        label = new QLabel("Would you like to reset your settings?");
    }

    QVBoxLayout* layout = new QVBoxLayout;

    layout->addWidget(label);

    QRadioButton* option1 = new QRadioButton("Reset all my settings");
    QRadioButton* option2 = new QRadioButton("Reset my settings but keep essential info");
    QRadioButton* option3 = new QRadioButton("Continue with my current settings");
    QLabel* crashReportLabel = nullptr;

    if (ual.isCrashMonitorStarted()) {
        crashReportLabel = new QLabel("To help us with debugging, you can enable automatic crash reports.\n"
                                      "They'll only be seen by developers trusted by the Overte e.V. organization,\n"
                                      "and will only be used for improving the code.");
    } else {
        crashReportLabel = new QLabel("Unfortunately, crash reporting isn't built into this release.");
    }

    QCheckBox* crashReportCheckbox = new QCheckBox("Enable automatic crash reporting");


    crashReportCheckbox->setChecked(ual.isCrashReportingEnabled());
    crashReportCheckbox->setEnabled(ual.isCrashMonitorStarted());

    option3->setChecked(true);
    layout->addWidget(option1);
    layout->addWidget(option2);
    layout->addWidget(option3);
    layout->addSpacing(12);

    layout->addWidget(crashReportLabel);
    layout->addWidget(crashReportCheckbox);
    layout->addSpacing(12);
    layout->addStretch();

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(buttons);
    crashDialog.connect(buttons, SIGNAL(accepted()), SLOT(accept()));

    crashDialog.setLayout(layout);

    int result = crashDialog.exec();

    if (result == QDialog::Accepted) {
        if (option1->isChecked()) {
            return CrashRecoveryHandler::DELETE_INTERFACE_INI;
        }
        if (option2->isChecked()) {
            return CrashRecoveryHandler::RETAIN_IMPORTANT_INFO;
        }
    }

    ual.setCrashReportingEnabled(crashReportCheckbox->isChecked());

    // Dialog cancelled or "do nothing" option chosen
    return CrashRecoveryHandler::DO_NOTHING;
}

void CrashRecoveryHandler::handleCrash(CrashRecoveryHandler::Action action) {
    if (action != CrashRecoveryHandler::DELETE_INTERFACE_INI && action != CrashRecoveryHandler::RETAIN_IMPORTANT_INFO) {
        // CrashRecoveryHandler::DO_NOTHING or unexpected value
        return;
    }

    Settings settings;

    const QString ADDRESS_MANAGER_GROUP = "AddressManager";
    const QString ADDRESS_KEY = "address";
    const QString AVATAR_GROUP = "Avatar";
    const QString DISPLAY_NAME_KEY = "displayName";
    const QString FULL_AVATAR_URL_KEY = "fullAvatarURL";
    const QString FULL_AVATAR_MODEL_NAME_KEY = "fullAvatarModelName";
    const QString TUTORIAL_COMPLETE_FLAG_KEY = "tutorialComplete";

    QString displayName;
    QUrl fullAvatarURL;
    QString fullAvatarModelName;
    QUrl address;
    bool tutorialComplete = false;

    if (action == CrashRecoveryHandler::RETAIN_IMPORTANT_INFO) {
        // Read avatar info

        // Location and orientation
        settings.beginGroup(ADDRESS_MANAGER_GROUP);
        address = settings.value(ADDRESS_KEY).toUrl();
        settings.endGroup();

        // Display name and avatar
        settings.beginGroup(AVATAR_GROUP);
        displayName = settings.value(DISPLAY_NAME_KEY).toString();
        fullAvatarURL = settings.value(FULL_AVATAR_URL_KEY).toUrl();
        fullAvatarModelName = settings.value(FULL_AVATAR_MODEL_NAME_KEY).toString();
        settings.endGroup();

        // Tutorial complete
        tutorialComplete = settings.value(TUTORIAL_COMPLETE_FLAG_KEY).toBool();
    }

    // Reset everything
    settings.clear();

    if (action == CrashRecoveryHandler::RETAIN_IMPORTANT_INFO) {
        // Write avatar info

        // Location and orientation
        settings.beginGroup(ADDRESS_MANAGER_GROUP);
        settings.setValue(ADDRESS_KEY, address);
        settings.endGroup();

        // Display name and avatar
        settings.beginGroup(AVATAR_GROUP);
        settings.setValue(DISPLAY_NAME_KEY, displayName);
        settings.setValue(FULL_AVATAR_URL_KEY, fullAvatarURL);
        settings.setValue(FULL_AVATAR_MODEL_NAME_KEY, fullAvatarModelName);
        settings.endGroup();

        // Tutorial complete
        settings.setValue(TUTORIAL_COMPLETE_FLAG_KEY, tutorialComplete);
    }
}

