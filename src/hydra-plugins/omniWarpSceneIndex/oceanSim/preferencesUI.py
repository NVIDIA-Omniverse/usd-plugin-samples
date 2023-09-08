# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'preferencesUI.ui'
##
## Created by: Qt User Interface Compiler version 5.15.2
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide2.QtCore import *
from PySide2.QtGui import *
from PySide2.QtWidgets import *


class Ui_Preferences(object):
    def setupUi(self, Ocean_Simulation_Settings):
        if not Ocean_Simulation_Settings.objectName():
            Ocean_Simulation_Settings.setObjectName(u"Ocean_Simulation_Settings")
        Ocean_Simulation_Settings.resize(295, 99)
        self.verticalLayout = QVBoxLayout(Ocean_Simulation_Settings)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.prefsOverButtonsLayout = QVBoxLayout()
        self.prefsOverButtonsLayout.setObjectName(u"prefsOverButtonsLayout")
        self.horizontalLayout_3 = QHBoxLayout()
        self.horizontalLayout_3.setObjectName(u"horizontalLayout_3")
        self.scaleLabel = QLabel(Ocean_Simulation_Settings)
        self.scaleLabel.setObjectName(u"scaleLabel")

        self.horizontalLayout_3.addWidget(self.scaleLabel)

        self.horizontalSpacer_2a = QSpacerItem(40, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout_3.addItem(self.horizontalSpacer_2a)

        self.scaleSpinBox = QDoubleSpinBox(Ocean_Simulation_Settings)
        self.scaleSpinBox.setObjectName(u"scaleSpinBox")
        self.scaleSpinBox.setDecimals(2)
        self.scaleSpinBox.setMinimum(0.000000000000000)
        self.scaleSpinBox.setValue(1.000000000000000)

        self.horizontalLayout_3.addWidget(self.scaleSpinBox)


        self.prefsOverButtonsLayout.addLayout(self.horizontalLayout_3)

        self.horizontalLayout_4 = QHBoxLayout()
        self.horizontalLayout_4.setObjectName(u"horizontalLayout_4")
        self.directionLabel = QLabel(Ocean_Simulation_Settings)
        self.directionLabel.setObjectName(u"directionLabel")

        self.horizontalLayout_4.addWidget(self.directionLabel)

        self.horizontalSpacer_2b = QSpacerItem(26, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout_4.addItem(self.horizontalSpacer_2b)

        self.directionSpinBox = QDoubleSpinBox(Ocean_Simulation_Settings)
        self.directionSpinBox.setObjectName(u"directionSpinBox")
        self.directionSpinBox.setDecimals(2)
        self.directionSpinBox.setMinimum(0.000000000000000)
        self.directionSpinBox.setValue(0.000000000000000)

        self.horizontalLayout_4.addWidget(self.directionSpinBox)


        self.prefsOverButtonsLayout.addLayout(self.horizontalLayout_4)

        self.horizontalLayout_5 = QHBoxLayout()
        self.horizontalLayout_5.setObjectName(u"horizontalLayout_5")
        self.windSpeedLabel = QLabel(Ocean_Simulation_Settings)
        self.windSpeedLabel.setObjectName(u"windSpeedLabel")

        self.horizontalLayout_5.addWidget(self.windSpeedLabel)

        self.horizontalSpacer_2c = QSpacerItem(24, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout_5.addItem(self.horizontalSpacer_2c)

        self.windSpeedSpinBox = QDoubleSpinBox(Ocean_Simulation_Settings)
        self.windSpeedSpinBox.setObjectName(u"windSpeedSpinBox")
        self.windSpeedSpinBox.setDecimals(2)
        self.windSpeedSpinBox.setMinimum(0.000000000000000)
        self.windSpeedSpinBox.setValue(10.000000000000000)

        self.horizontalLayout_5.addWidget(self.windSpeedSpinBox)


        self.prefsOverButtonsLayout.addLayout(self.horizontalLayout_5)

        self.horizontalLayout_6 = QHBoxLayout()
        self.horizontalLayout_6.setObjectName(u"horizontalLayout_6")
        self.waterDepthLabel = QLabel(Ocean_Simulation_Settings)
        self.waterDepthLabel.setObjectName(u"waterDepthLabel")

        self.horizontalLayout_6.addWidget(self.waterDepthLabel)

        self.horizontalSpacer_2d = QSpacerItem(24, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout_6.addItem(self.horizontalSpacer_2d)

        self.waterDepthSpinBox = QDoubleSpinBox(Ocean_Simulation_Settings)
        self.waterDepthSpinBox.setObjectName(u"waterDepthSpinBox")
        self.waterDepthSpinBox.setDecimals(2)
        self.waterDepthSpinBox.setMinimum(0.000000000000000)
        self.waterDepthSpinBox.setValue(50.000000000000000)

        self.horizontalLayout_6.addWidget(self.waterDepthSpinBox)


        self.prefsOverButtonsLayout.addLayout(self.horizontalLayout_6)

        self.horizontalLayout_7 = QHBoxLayout()
        self.horizontalLayout_7.setObjectName(u"horizontalLayout_7")
        self.waveAmplitudeLabel = QLabel(Ocean_Simulation_Settings)
        self.waveAmplitudeLabel.setObjectName(u"waveAmplitudeLabel")

        self.horizontalLayout_7.addWidget(self.waveAmplitudeLabel)

        self.horizontalSpacer_2e = QSpacerItem(21, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout_7.addItem(self.horizontalSpacer_2e)

        self.waveAmplitudeSpinBox = QDoubleSpinBox(Ocean_Simulation_Settings)
        self.waveAmplitudeSpinBox.setObjectName(u"waveAmplitudeSpinBox")
        self.waveAmplitudeSpinBox.setDecimals(2)
        self.waveAmplitudeSpinBox.setMinimum(0.000000000000000)
        self.waveAmplitudeSpinBox.setValue(1.500000000000000)

        self.horizontalLayout_7.addWidget(self.waveAmplitudeSpinBox)


        self.prefsOverButtonsLayout.addLayout(self.horizontalLayout_7)

        self.horizontalLayout_8 = QHBoxLayout()
        self.horizontalLayout_8.setObjectName(u"horizontalLayout_8")
        self.waveDirectionalityLabel = QLabel(Ocean_Simulation_Settings)
        self.waveDirectionalityLabel.setObjectName(u"waveDirectionalityLabel")

        self.horizontalLayout_8.addWidget(self.waveDirectionalityLabel)

        self.horizontalSpacer_2f = QSpacerItem(17, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout_8.addItem(self.horizontalSpacer_2f)

        self.waveDirectionalitySpinBox = QDoubleSpinBox(Ocean_Simulation_Settings)
        self.waveDirectionalitySpinBox.setObjectName(u"waveDirectionalitySpinBox")
        self.waveDirectionalitySpinBox.setMinimum(0.000000000000000)
        self.waveDirectionalitySpinBox.setValue(0.000000000000000)

        self.horizontalLayout_8.addWidget(self.waveDirectionalitySpinBox)


        self.prefsOverButtonsLayout.addLayout(self.horizontalLayout_8)

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)

        self.prefsOverButtonsLayout.addItem(self.verticalSpacer)

        self.line = QFrame(Ocean_Simulation_Settings)
        self.line.setObjectName(u"line")
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.prefsOverButtonsLayout.addWidget(self.line)

        self.horizontalLayout_2 = QHBoxLayout()
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.horizontalSpacer = QSpacerItem(40, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout_2.addItem(self.horizontalSpacer)

        self.buttonBox = QDialogButtonBox(Ocean_Simulation_Settings)
        self.buttonBox.setObjectName(u"buttonBox")
        self.buttonBox.setStandardButtons(QDialogButtonBox.Apply|QDialogButtonBox.Cancel|QDialogButtonBox.Ok)

        self.horizontalLayout_2.addWidget(self.buttonBox)


        self.prefsOverButtonsLayout.addLayout(self.horizontalLayout_2)


        self.verticalLayout.addLayout(self.prefsOverButtonsLayout)


        self.retranslateUi(Ocean_Simulation_Settings)

        QMetaObject.connectSlotsByName(Ocean_Simulation_Settings)
    # setupUi

    def retranslateUi(self, Ocean_Simulation_Settings):
        Ocean_Simulation_Settings.setWindowTitle(QCoreApplication.translate("Preferences", u"Ocean Simulation Settings", None))
        Ocean_Simulation_Settings.setProperty("comment", QCoreApplication.translate("Preferences", u"\n"
"     Copyright 2020 Pixar                                                                   \n"
"                                                                                            \n"
"     Licensed under the Apache License, Version 2.0 (the \"Apache License\")      \n"
"     with the following modification; you may not use this file except in                   \n"
"     compliance with the Apache License and the following modification to it:               \n"
"     Section 6. Trademarks. is deleted and replaced with:                                   \n"
"                                                                                            \n"
"     6. Trademarks. This License does not grant permission to use the trade                 \n"
"        names, trademarks, service marks, or product names of the Licensor                  \n"
"        and its affiliates, except as required to comply with Section 4(c) of               \n"
"        the License and to reproduce the content of the NOTI"
                        "CE file.                        \n"
"                                                                                            \n"
"     You may obtain a copy of the Apache License at                                         \n"
"                                                                                            \n"
"         http://www.apache.org/licenses/LICENSE-2.0                                         \n"
"                                                                                            \n"
"     Unless required by applicable law or agreed to in writing, software                    \n"
"     distributed under the Apache License with the above modification is                    \n"
"     distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY   \n"
"     KIND, either express or implied. See the Apache License for the specific               \n"
"     language governing permissions and limitations under the Apache License.               \n"
"  ", None))
        self.scaleLabel.setText(QCoreApplication.translate("Preferences", u"Scale", None))
        self.directionLabel.setText(QCoreApplication.translate("Preferences", u"Direction", None))
        self.windSpeedLabel.setText(QCoreApplication.translate("Preferences", u"Wind Speed", None))
        self.waterDepthLabel.setText(QCoreApplication.translate("Preferences", u"Water Depth", None))
        self.waveAmplitudeLabel.setText(QCoreApplication.translate("Preferences", u"Wave Amplitude", None))
        self.waveDirectionalityLabel.setText(QCoreApplication.translate("Preferences", u"Wave Directionality", None))
    # retranslateUi

