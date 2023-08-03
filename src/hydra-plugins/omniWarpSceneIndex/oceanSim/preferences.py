#
# Copyright 2016 Pixar
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
#

from pxr.Usdviewq.qt import QtCore, QtGui, QtWidgets
from pxr.WarpSceneIndexPlugin.preferencesUI import Ui_Preferences

class Preferences(QtWidgets.QDialog):
    def __init__(self, parent, attr):
        super(Preferences, self).__init__(parent)
        self._ui = Ui_Preferences()
        self._ui.setupUi(self)
        self._attr = attr


        metadata = self._attr.GetMetadata("customData")

        self._ui.scaleSpinBox.setValue(metadata["scale"])
        self._ui.directionSpinBox.setValue(metadata["direction"])
        self._ui.windSpeedSpinBox.setValue(metadata["wind_speed"])
        self._ui.waterDepthSpinBox.setValue(metadata["water_depth"])
        self._ui.waveAmplitudeSpinBox.setValue(metadata["wave_amplitude"])
        self._ui.waveDirectionalitySpinBox.setValue(metadata["wave_directionality"])

        self._ui.buttonBox.clicked.connect(self._buttonBoxButtonClicked)

    def _apply(self):
        self._attr.SetMetadataByDictKey('customData', 'scale', self._ui.scaleSpinBox.value())
        self._attr.SetMetadataByDictKey('customData', 'direction', self._ui.directionSpinBox.value())
        self._attr.SetMetadataByDictKey('customData', 'wind_speed', self._ui.windSpeedSpinBox.value())
        self._attr.SetMetadataByDictKey('customData', 'water_depth', self._ui.waterDepthSpinBox.value())
        self._attr.SetMetadataByDictKey('customData', 'wave_amplitude', self._ui.waveAmplitudeSpinBox.value())
        self._attr.SetMetadataByDictKey('customData', 'wave_directionality', self._ui.waveDirectionalitySpinBox.value())
 
    def _buttonBoxButtonClicked(self, button):
        role = self._ui.buttonBox.buttonRole(button)
        Roles = QtWidgets.QDialogButtonBox.ButtonRole
        if role == Roles.AcceptRole or role == Roles.ApplyRole:
            self._apply()
        if role == Roles.AcceptRole or role == Roles.RejectRole:
            self.close()
