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


class BookmarksList(QListWidget):

    changed = pyqtSignal()
    bookmark_activated = pyqtSignal(object)

    def __init__(self, parent=None):
        QListWidget.__init__(self, parent)
        self.setDragEnabled(True)
        self.setDragDropMode(self.InternalMove)
        self.setDefaultDropAction(Qt.MoveAction)
        self.setAlternatingRowColors(True)
        self.setStyleSheet('QListView::item { padding: 0.5ex }')
        self.viewport().setAcceptDrops(True)
        self.setDropIndicatorShown(True)
        self.setContextMenuPolicy(Qt.ActionsContextMenu)
        self.ac_sort = QAction(_('Sort by name'), self)
        self.addAction(self.ac_sort)
        self.ac_sort_pos = QAction(_('Sort by position in book'), self)
        self.addAction(self.ac_sort_pos)
        self.ac_delete = QAction(QIcon(I('trash.png')), _('Remove this bookmark'), self)
        self.addAction(self.ac_delete)
        self.ac_edit = QAction(QIcon(I('edit_input.png')), _('Edit this bookmark'), self)
        self.addAction(self.ac_edit)

    def dropEvent(self, ev):
        QListWidget.dropEvent(self, ev)
        if ev.isAccepted():
            self.changed.emit()

    def keyPressEvent(self, ev):
        k = ev.key()
        if k in (Qt.Key_Enter, Qt.Key_Return):
            i = self.currentItem()
            if i is not None:
                self.bookmark_activated.emit(i)
                ev.accept()
                return
        if k in (Qt.Key_Delete, Qt.Key_Backspace):
            i = self.currentItem()
            if i is not None:
                self.ac_delete.trigger()
                ev.accept()
                return
        return QListWidget.keyPressEvent(self, ev)


class BookmarkManager(QWidget):

    edited = pyqtSignal(object)
    activated = pyqtSignal(object)
    create_requested = pyqtSignal()

    def __init__(self, parent):
        QWidget.__init__(self, parent)
        self.l = QGridLayout(self)
        self.l.setContentsMargins(0, 0, 0, 0)
        self.setLayout(self.l)

        self.bookmarks_list = BookmarksList(self)
        self.bookmarks_list.itemChanged.connect(self.item_changed)
        self.l.addWidget(self.bookmarks_list, 0, 0, 1, -1)
        self.bookmarks_list.itemClicked.connect(self.item_activated)
        self.bookmarks_list.bookmark_activated.connect(self.item_activated)
        self.bookmarks_list.changed.connect(lambda : self.edited.emit(self.get_bookmarks()))
        self.bookmarks_list.ac_edit.triggered.connect(self.edit_bookmark)
        self.bookmarks_list.ac_sort.triggered.connect(self.sort_by_name)
        self.bookmarks_list.ac_sort_pos.triggered.connect(self.sort_by_pos)
        self.bookmarks_list.ac_delete.triggered.connect(self.delete_bookmark)

        self.la = QLabel(_('Double click to edit and drag-and-drop to re-order the bookmarks'))
        self.la.setWordWrap(True)
        self.l.addWidget(self.la, self.l.rowCount(), 0, 1, -1)

        self.button_new = QPushButton(QIcon(I('bookmarks.png')), _('&New'), self)
        self.button_new.clicked.connect(self.create_requested)
        self.button_new.setToolTip(_('Create a new bookmark at the current location'))
        self.l.addWidget(self.button_new)

        self.button_delete = QPushButton(QIcon(I('trash.png')), _('&Remove'), self)
        self.button_delete.setToolTip(_('Remove the currently selected bookmark'))
        self.button_delete.clicked.connect(self.delete_bookmark)
        self.l.addWidget(self.button_delete, self.l.rowCount() - 1, 1)

        self.button_sort_name = QPushButton(_('Sort by &name'), self)
        self.button_sort_name.setToolTip(_('Sort bookmarks by name'))
        self.button_sort_name.clicked.connect(self.sort_by_name)
        self.l.addWidget(self.button_sort_name)

        self.button_sort_pos = QPushButton(_('Sort by &position'), self)
        self.button_sort_pos.setToolTip(_('Sort bookmarks by position in book'))
        self.button_sort_pos.clicked.connect(self.sort_by_pos)
        self.l.addWidget(self.button_sort_pos, self.l.rowCount() - 1, 1)

        self.button_export = QPushButton(QIcon(I('back.png')), _('E&xport'), self)
        self.button_export.clicked.connect(self.export_bookmarks)
        self.l.addWidget(self.button_export)

        self.button_import = QPushButton(QIcon(I('forward.png')), _('&Import'), self)
        self.button_import.clicked.connect(self.import_bookmarks)
        self.l.addWidget(self.button_import, self.l.rowCount() - 1, 1)

    def item_activated(self, item):
        bm = self.item_to_bm(item)
        self.activated.emit(bm)

    def set_bookmarks(self, bookmarks=()):
        self.bookmarks_list.clear()
        for bm in bookmarks:
            if bm['title'] != 'calibre_current_page_bookmark':
                i = QListWidgetItem(bm['title'])
                i.setData(Qt.UserRole, self.bm_to_item(bm))
                i.setFlags(i.flags() | Qt.ItemIsEditable)
                self.bookmarks_list.addItem(i)
        if self.bookmarks_list.count() > 0:
            self.bookmarks_list.setCurrentItem(self.bookmarks_list.item(0), QItemSelectionModel.ClearAndSelect)

    def set_current_bookmark(self, bm):
        for i, q in enumerate(self):
            if bm == q:
                l = self.bookmarks_list
                item = l.item(i)
                l.setCurrentItem(item, QItemSelectionModel.ClearAndSelect)
                l.scrollToItem(item)

    def __iter__(self):
        for i in xrange(self.bookmarks_list.count()):
            yield self.item_to_bm(self.bookmarks_list.item(i))

    def item_changed(self, item):
        self.bookmarks_list.blockSignals(True)
        title = unicode(item.data(Qt.DisplayRole))
        if not title:
            title = _('Unknown')
            item.setData(Qt.DisplayRole, title)
        bm = self.item_to_bm(item)
        bm['title'] = title
        item.setData(Qt.UserRole, self.bm_to_item(bm))
        self.bookmarks_list.blockSignals(False)
        self.edited.emit(self.get_bookmarks())

    def delete_bookmark(self):
        row = self.bookmarks_list.currentRow()
        if row > -1:
            self.bookmarks_list.takeItem(row)
            self.edited.emit(self.get_bookmarks())

    def edit_bookmark(self):
        item = self.bookmarks_list.currentItem()
        if item is not None:
            self.bookmarks_list.editItem(item)

    def sort_by_name(self):
        bm = self.get_bookmarks()
        bm.sort(key=lambda x: sort_key(x['title']))
        self.set_bookmarks(bm)
        self.edited.emit(bm)

    def sort_by_pos(self):
        from calibre.ebooks.epub.cfi.parse import cfi_sort_key

        def pos_key(b):
            if b.get('type', None) == 'cfi':
                return b['spine'], cfi_sort_key(b['pos'])
            return (None, None)
        bm = self.get_bookmarks()
        bm.sort(key=pos_key)
        self.set_bookmarks(bm)
        self.edited.emit(bm)

    def bm_to_item(self, bm):
        return bm.copy()

    def item_to_bm(self, item):
        return item.data(Qt.UserRole).copy()

    def get_bookmarks(self):
        return list(self)

    def export_bookmarks(self):
        filename = choose_save_file(
            self, 'export-viewer-bookmarks', _('Export bookmarks'),
            filters=[(_('Saved bookmarks'), ['calibre-bookmarks'])], all_files=False, initial_filename='bookmarks.calibre-bookmarks')
        if filename:
            with lopen(filename, 'wb') as fileobj:
                fileobj.write(json.dumps(self.get_bookmarks(), indent=True))

    def import_bookmarks(self):
        files = choose_files(self, 'export-viewer-bookmarks', _('Import bookmarks'),
            filters=[(_('Saved bookmarks'), ['calibre-bookmarks'])], all_files=False, select_only_single_file=True)
        if not files:
            return
        filename = files[0]

        imported = None
        with lopen(filename, 'rb') as fileobj:
            imported = json.load(fileobj)

        if imported is not None:
            bad = False
            try:
                for bm in imported:
                    if 'title' not in bm:
                        bad = True
                        break
            except Exception:
                pass

            if not bad:
                bookmarks = self.get_bookmarks()
                for bm in imported:
                    if bm not in bookmarks:
                        bookmarks.append(bm)
                self.set_bookmarks([bm for bm in bookmarks if bm['title'] != 'calibre_current_page_bookmark'])
                self.edited.emit(self.get_bookmarks())