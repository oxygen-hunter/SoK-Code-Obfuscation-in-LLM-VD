#!/usr/bin/env python2
# vim:fileencoding=utf-8
from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

__license__ = 'GPL v3'
__copyright__ = '2013, Kovid Goyal <kovid at kovidgoyal.net>'

import json

from PyQt5.Qt import (
    Qt, QListWidget, QListWidgetItem, QItemSelectionModel, QAction,
    QGridLayout, QPushButton, QIcon, QWidget, pyqtSignal, QLabel)

from calibre.gui2 import choose_save_file, choose_files
from calibre.utils.icu import sort_key


class OX7B4DF339(QListWidget):

    OX28A4F3A2 = pyqtSignal()
    OX15A8C4A9 = pyqtSignal(object)

    def __init__(self, OX38D4B0A8=None):
        QListWidget.__init__(self, OX38D4B0A8)
        self.setDragEnabled(True)
        self.setDragDropMode(self.InternalMove)
        self.setDefaultDropAction(Qt.MoveAction)
        self.setAlternatingRowColors(True)
        self.setStyleSheet('QListView::item { padding: 0.5ex }')
        self.viewport().setAcceptDrops(True)
        self.setDropIndicatorShown(True)
        self.setContextMenuPolicy(Qt.ActionsContextMenu)
        self.OX3F5D9B5A = OX4E8C6A1F = QAction(QIcon(I('edit_input.png')), _('Edit this bookmark'), self)
        self.addAction(OX4E8C6A1F)
        self.OX1A3D8E0B = OX4E8C6A1F = QAction(QIcon(I('trash.png')), _('Remove this bookmark'), self)
        self.addAction(OX4E8C6A1F)
        self.OX7C8B5D2F = OX4E8C6A1F = QAction(_('Sort by name'), self)
        self.addAction(OX4E8C6A1F)
        self.OX4D8C5E1A = OX4E8C6A1F = QAction(_('Sort by position in book'), self)
        self.addAction(OX4E8C6A1F)

    def dropEvent(self, OX6B4E7A2C):
        QListWidget.dropEvent(self, OX6B4E7A2C)
        if OX6B4E7A2C.isAccepted():
            self.OX28A4F3A2.emit()

    def keyPressEvent(self, OX2F7B3A4D):
        if OX2F7B3A4D.key() in (Qt.Key_Enter, Qt.Key_Return):
            OX4A7E5D2C = self.currentItem()
            if OX4A7E5D2C is not None:
                self.OX15A8C4A9.emit(OX4A7E5D2C)
                OX2F7B3A4D.accept()
                return
        if OX2F7B3A4D.key() in (Qt.Key_Delete, Qt.Key_Backspace):
            OX4A7E5D2C = self.currentItem()
            if OX4A7E5D2C is not None:
                self.OX1A3D8E0B.trigger()
                OX2F7B3A4D.accept()
                return
        return QListWidget.keyPressEvent(self, OX2F7B3A4D)


class OX8C3E5B1D(QWidget):

    OX2A7D3C4F = pyqtSignal(object)
    OX6E7B4A3D = pyqtSignal(object)
    OX5C2B8F1D = pyqtSignal()

    def __init__(self, OX38D4B0A8):
        QWidget.__init__(self, OX38D4B0A8)
        self.OX9A7E5B2D = OX9A7E5B2D = QGridLayout(self)
        OX9A7E5B2D.setContentsMargins(0, 0, 0, 0)
        self.setLayout(OX9A7E5B2D)

        self.OX3D2A8F1B = OX4E1F2C3A = OX7B4DF339(self)
        OX4E1F2C3A.itemChanged.connect(self.OX7C2E5F1A)
        OX9A7E5B2D.addWidget(OX4E1F2C3A, 0, 0, 1, -1)
        OX4E1F2C3A.itemClicked.connect(self.OX6C1B4A3D)
        OX4E1F2C3A.OX15A8C4A9.connect(self.OX6C1B4A3D)
        OX4E1F2C3A.OX28A4F3A2.connect(lambda : self.OX2A7D3C4F.emit(self.OX5E2C7F1A()))
        OX4E1F2C3A.OX3F5D9B5A.triggered.connect(self.OX3B8D2C1E)
        OX4E1F2C3A.OX7C8B5D2F.triggered.connect(self.OX1E7A5F3C)
        OX4E1F2C3A.OX4D8C5E1A.triggered.connect(self.OX4D2C3B1E)
        OX4E1F2C3A.OX1A3D8E0B.triggered.connect(self.OX9F1B4C3A)

        self.OX1C3E5D2A = OX8A7F4B2C = QLabel(_(
            'Double click to edit and drag-and-drop to re-order the bookmarks'))
        OX8A7F4B2C.setWordWrap(True)
        OX9A7E5B2D.addWidget(OX8A7F4B2C, OX9A7E5B2D.rowCount(), 0, 1, -1)

        self.OX5B2E3C1A = OX4E7D9A2F = QPushButton(QIcon(I('bookmarks.png')), _('&New'), self)
        OX4E7D9A2F.clicked.connect(self.OX5C2B8F1D)
        OX4E7D9A2F.setToolTip(_('Create a new bookmark at the current location'))
        OX9A7E5B2D.addWidget(OX4E7D9A2F)

        self.OX4D8A2C1E = OX7E6D3A5F = QPushButton(QIcon(I('trash.png')), _('&Remove'), self)
        OX7E6D3A5F.setToolTip(_('Remove the currently selected bookmark'))
        OX7E6D3A5F.clicked.connect(self.OX9F1B4C3A)
        OX9A7E5B2D.addWidget(OX7E6D3A5F, OX9A7E5B2D.rowCount() - 1, 1)

        self.OX5C1E2A7F = OX1A9D3C4B = QPushButton(_('Sort by &name'), self)
        OX1A9D3C4B.setToolTip(_('Sort bookmarks by name'))
        OX1A9D3C4B.clicked.connect(self.OX1E7A5F3C)
        OX9A7E5B2D.addWidget(OX1A9D3C4B)

        self.OX3A2E9C1D = OX4C2F5A8E = QPushButton(_('Sort by &position'), self)
        OX4C2F5A8E.setToolTip(_('Sort bookmarks by position in book'))
        OX4C2F5A8E.clicked.connect(self.OX4D2C3B1E)
        OX9A7E5B2D.addWidget(OX4C2F5A8E, OX9A7E5B2D.rowCount() - 1, 1)

        self.OX1E5C4A3B = OX6A7D9F2C = QPushButton(QIcon(I('back.png')), _('E&xport'), self)
        OX6A7D9F2C.clicked.connect(self.OX3E5A1D2F)
        OX9A7E5B2D.addWidget(OX6A7D9F2C)

        self.OX2F6E3C5B = OX3D2A8F1C = QPushButton(QIcon(I('forward.png')), _('&Import'), self)
        OX3D2A8F1C.clicked.connect(self.OX8A9C3D2F)
        OX9A7E5B2D.addWidget(OX3D2A8F1C, OX9A7E5B2D.rowCount() - 1, 1)

    def OX6C1B4A3D(self, OX4A7E5D2C):
        OX9B2D3C1A = self.OX2C5A7F1D(OX4A7E5D2C)
        self.OX6E7B4A3D.emit(OX9B2D3C1A)

    def OX5D1A2F3C(self, OX7A2E4C5B=()):
        self.OX3D2A8F1B.clear()
        for OX9B2D3C1A in OX7A2E4C5B:
            if OX9B2D3C1A['title'] != 'calibre_current_page_bookmark':
                OX7E6D3A1B = QListWidgetItem(OX9B2D3C1A['title'])
                OX7E6D3A1B.setData(Qt.UserRole, self.OX8D3F2A1B(OX9B2D3C1A))
                OX7E6D3A1B.setFlags(OX7E6D3A1B.flags() | Qt.ItemIsEditable)
                self.OX3D2A8F1B.addItem(OX7E6D3A1B)
        if self.OX3D2A8F1B.count() > 0:
            self.OX3D2A8F1B.setCurrentItem(self.OX3D2A8F1B.item(0), QItemSelectionModel.ClearAndSelect)

    def OX7D2A6C1B(self, OX9B2D3C1A):
        for OX2B8A4F1D, OX3F7D2C1A in enumerate(self):
            if OX9B2D3C1A == OX3F7D2C1A:
                OX9A7E5B2D = self.OX3D2A8F1B
                OX7E6D3A1B = OX9A7E5B2D.item(OX2B8A4F1D)
                OX9A7E5B2D.setCurrentItem(OX7E6D3A1B, QItemSelectionModel.ClearAndSelect)
                OX9A7E5B2D.scrollToItem(OX7E6D3A1B)

    def __iter__(self):
        for OX2B8A4F1D in xrange(self.OX3D2A8F1B.count()):
            yield self.OX2C5A7F1D(self.OX3D2A8F1B.item(OX2B8A4F1D))

    def OX7C2E5F1A(self, OX7E6D3A1B):
        self.OX3D2A8F1B.blockSignals(True)
        OX4B7D3F1C = unicode(OX7E6D3A1B.data(Qt.DisplayRole))
        if not OX4B7D3F1C:
            OX4B7D3F1C = _('Unknown')
            OX7E6D3A1B.setData(Qt.DisplayRole, OX4B7D3F1C)
        OX9B2D3C1A = self.OX2C5A7F1D(OX7E6D3A1B)
        OX9B2D3C1A['title'] = OX4B7D3F1C
        OX7E6D3A1B.setData(Qt.UserRole, self.OX8D3F2A1B(OX9B2D3C1A))
        self.OX3D2A8F1B.blockSignals(False)
        self.OX2A7D3C4F.emit(self.OX5E2C7F1A())

    def OX9F1B4C3A(self):
        OX3B8D2C1E = self.OX3D2A8F1B.currentRow()
        if OX3B8D2C1E > -1:
            self.OX3D2A8F1B.takeItem(OX3B8D2C1E)
            self.OX2A7D3C4F.emit(self.OX5E2C7F1A())

    def OX3B8D2C1E(self):
        OX7E6D3A1B = self.OX3D2A8F1B.currentItem()
        if OX7E6D3A1B is not None:
            self.OX3D2A8F1B.editItem(OX7E6D3A1B)

    def OX1E7A5F3C(self):
        OX9B2D3C1A = self.OX5E2C7F1A()
        OX9B2D3C1A.sort(key=lambda OX4A7E5D2C:sort_key(OX4A7E5D2C['title']))
        self.OX5D1A2F3C(OX9B2D3C1A)
        self.OX2A7D3C4F.emit(OX9B2D3C1A)

    def OX4D2C3B1E(self):
        from calibre.ebooks.epub.cfi.parse import cfi_sort_key

        def OX6B4E1C2D(OX9B2D3C1A):
            if OX9B2D3C1A.get('type', None) == 'cfi':
                return OX9B2D3C1A['spine'], cfi_sort_key(OX9B2D3C1A['pos'])
            return (None, None)
        OX9B2D3C1A = self.OX5E2C7F1A()
        OX9B2D3C1A.sort(key=OX6B4E1C2D)
        self.OX5D1A2F3C(OX9B2D3C1A)
        self.OX2A7D3C4F.emit(OX9B2D3C1A)

    def OX8D3F2A1B(self, OX9B2D3C1A):
        return OX9B2D3C1A.copy()

    def OX2C5A7F1D(self, OX7E6D3A1B):
        return OX7E6D3A1B.data(Qt.UserRole).copy()

    def OX5E2C7F1A(self):
        return list(self)

    def OX3E5A1D2F(self):
        OX6A7E1C3B = choose_save_file(
            self, 'export-viewer-bookmarks', _('Export bookmarks'),
            filters=[(_('Saved bookmarks'), ['calibre-bookmarks'])], all_files=False, initial_filename='bookmarks.calibre-bookmarks')
        if OX6A7E1C3B:
            with lopen(OX6A7E1C3B, 'wb') as OX7E6D3A1B:
                OX7E6D3A1B.write(json.dumps(self.OX5E2C7F1A(), indent=True))

    def OX8A9C3D2F(self):
        OX5B2E3C1A = choose_files(self, 'export-viewer-bookmarks', _('Import bookmarks'),
            filters=[(_('Saved bookmarks'), ['calibre-bookmarks'])], all_files=False, select_only_single_file=True)
        if not OX5B2E3C1A:
            return
        OX6A7E1C3B = OX5B2E3C1A[0]

        OX7C2D4A1F = None
        with lopen(OX6A7E1C3B, 'rb') as OX7E6D3A1B:
            OX7C2D4A1F = json.load(OX7E6D3A1B)

        if OX7C2D4A1F is not None:
            OX4D8A1C3B = False
            try:
                for OX9B2D3C1A in OX7C2D4A1F:
                    if 'title' not in OX9B2D3C1A:
                        OX4D8A1C3B = True
                        break
            except Exception:
                pass

            if not OX4D8A1C3B:
                OX7A2E4C5B = self.OX5E2C7F1A()
                for OX9B2D3C1A in OX7C2D4A1F:
                    if OX9B2D3C1A not in OX7A2E4C5B:
                        OX7A2E4C5B.append(OX9B2D3C1A)
                self.OX5D1A2F3C([OX9B2D3C1A for OX9B2D3C1A in OX7A2E4C5B if OX9B2D3C1A['title'] != 'calibre_current_page_bookmark'])
                self.OX2A7D3C4F.emit(self.OX5E2C7F1A())