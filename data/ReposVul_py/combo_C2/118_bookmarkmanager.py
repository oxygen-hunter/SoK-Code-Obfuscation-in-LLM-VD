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
        self.ac_edit = ac = QAction(QIcon(I('edit_input.png')), _('Edit this bookmark'), self)
        self.addAction(ac)
        self.ac_delete = ac = QAction(QIcon(I('trash.png')), _('Remove this bookmark'), self)
        self.addAction(ac)
        self.ac_sort = ac = QAction(_('Sort by name'), self)
        self.addAction(ac)
        self.ac_sort_pos = ac = QAction(_('Sort by position in book'), self)
        self.addAction(ac)

    def dropEvent(self, ev):
        QListWidget.dropEvent(self, ev)
        if ev.isAccepted():
            self.changed.emit()

    def keyPressEvent(self, ev):
        dispatcher = None
        i = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                if ev.key() in (Qt.Key_Enter, Qt.Key_Return):
                    i = self.currentItem()
                    if i is not None:
                        dispatcher = "ACTIVATE_BOOKMARK"
                    else:
                        dispatcher = "DELETE_CHECK"
                else:
                    dispatcher = "DELETE_CHECK"
            elif dispatcher == "ACTIVATE_BOOKMARK":
                self.bookmark_activated.emit(i)
                ev.accept()
                dispatcher = "END"
            elif dispatcher == "DELETE_CHECK":
                if ev.key() in (Qt.Key_Delete, Qt.Key_Backspace):
                    dispatcher = "DELETE_BOOKMARK"
                else:
                    dispatcher = "DEFAULT_ACTION"
            elif dispatcher == "DELETE_BOOKMARK":
                i = self.currentItem()
                if i is not None:
                    self.ac_delete.trigger()
                    ev.accept()
                dispatcher = "END"
            elif dispatcher == "DEFAULT_ACTION":
                QListWidget.keyPressEvent(self, ev)
                dispatcher = "END"


class BookmarkManager(QWidget):

    edited = pyqtSignal(object)
    activated = pyqtSignal(object)
    create_requested = pyqtSignal()

    def __init__(self, parent):
        QWidget.__init__(self, parent)
        self.l = l = QGridLayout(self)
        l.setContentsMargins(0, 0, 0, 0)
        self.setLayout(l)

        self.bookmarks_list = bl = BookmarksList(self)
        bl.itemChanged.connect(self.item_changed)
        l.addWidget(bl, 0, 0, 1, -1)
        bl.itemClicked.connect(self.item_activated)
        bl.bookmark_activated.connect(self.item_activated)
        bl.changed.connect(lambda : self.edited.emit(self.get_bookmarks()))
        bl.ac_edit.triggered.connect(self.edit_bookmark)
        bl.ac_sort.triggered.connect(self.sort_by_name)
        bl.ac_sort_pos.triggered.connect(self.sort_by_pos)
        bl.ac_delete.triggered.connect(self.delete_bookmark)

        self.la = la = QLabel(_(
            'Double click to edit and drag-and-drop to re-order the bookmarks'))
        la.setWordWrap(True)
        l.addWidget(la, l.rowCount(), 0, 1, -1)

        self.button_new = b = QPushButton(QIcon(I('bookmarks.png')), _('&New'), self)
        b.clicked.connect(self.create_requested)
        b.setToolTip(_('Create a new bookmark at the current location'))
        l.addWidget(b)

        self.button_delete = b = QPushButton(QIcon(I('trash.png')), _('&Remove'), self)
        b.setToolTip(_('Remove the currently selected bookmark'))
        b.clicked.connect(self.delete_bookmark)
        l.addWidget(b, l.rowCount() - 1, 1)

        self.button_delete = b = QPushButton(_('Sort by &name'), self)
        b.setToolTip(_('Sort bookmarks by name'))
        b.clicked.connect(self.sort_by_name)
        l.addWidget(b)

        self.button_delete = b = QPushButton(_('Sort by &position'), self)
        b.setToolTip(_('Sort bookmarks by position in book'))
        b.clicked.connect(self.sort_by_pos)
        l.addWidget(b, l.rowCount() - 1, 1)

        self.button_export = b = QPushButton(QIcon(I('back.png')), _('E&xport'), self)
        b.clicked.connect(self.export_bookmarks)
        l.addWidget(b)

        self.button_import = b = QPushButton(QIcon(I('forward.png')), _('&Import'), self)
        b.clicked.connect(self.import_bookmarks)
        l.addWidget(b, l.rowCount() - 1, 1)

    def item_activated(self, item):
        dispatcher = None
        bm = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                bm = self.item_to_bm(item)
                dispatcher = "ACTIVATE"
            elif dispatcher == "ACTIVATE":
                self.activated.emit(bm)
                dispatcher = "END"

    def set_bookmarks(self, bookmarks=()):
        dispatcher = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                self.bookmarks_list.clear()
                dispatcher = "CLEAR"
            elif dispatcher == "CLEAR":
                for bm in bookmarks:
                    if bm['title'] != 'calibre_current_page_bookmark':
                        dispatcher = "ADD_ITEM"
                        break
                else:
                    dispatcher = "SET_CURRENT"
            elif dispatcher == "ADD_ITEM":
                i = QListWidgetItem(bm['title'])
                i.setData(Qt.UserRole, self.bm_to_item(bm))
                i.setFlags(i.flags() | Qt.ItemIsEditable)
                self.bookmarks_list.addItem(i)
                dispatcher = "CLEAR"
            elif dispatcher == "SET_CURRENT":
                if self.bookmarks_list.count() > 0:
                    self.bookmarks_list.setCurrentItem(self.bookmarks_list.item(0), QItemSelectionModel.ClearAndSelect)
                dispatcher = "END"

    def set_current_bookmark(self, bm):
        dispatcher = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                for i, q in enumerate(self):
                    if bm == q:
                        dispatcher = "SCROLL_TO"
                        break
                else:
                    dispatcher = "END"
            elif dispatcher == "SCROLL_TO":
                l = self.bookmarks_list
                item = l.item(i)
                l.setCurrentItem(item, QItemSelectionModel.ClearAndSelect)
                l.scrollToItem(item)
                dispatcher = "END"

    def __iter__(self):
        dispatcher = None
        i = 0
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                while i < self.bookmarks_list.count():
                    yield self.item_to_bm(self.bookmarks_list.item(i))
                    i += 1
                dispatcher = "END"

    def item_changed(self, item):
        dispatcher = None
        title = None
        bm = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                self.bookmarks_list.blockSignals(True)
                dispatcher = "GET_TITLE"
            elif dispatcher == "GET_TITLE":
                title = unicode(item.data(Qt.DisplayRole))
                dispatcher = "CHECK_TITLE"
            elif dispatcher == "CHECK_TITLE":
                if not title:
                    title = _('Unknown')
                    item.setData(Qt.DisplayRole, title)
                dispatcher = "UPDATE_BM"
            elif dispatcher == "UPDATE_BM":
                bm = self.item_to_bm(item)
                bm['title'] = title
                item.setData(Qt.UserRole, self.bm_to_item(bm))
                dispatcher = "UNBLOCK_SIGNALS"
            elif dispatcher == "UNBLOCK_SIGNALS":
                self.bookmarks_list.blockSignals(False)
                self.edited.emit(self.get_bookmarks())
                dispatcher = "END"

    def delete_bookmark(self):
        dispatcher = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                row = self.bookmarks_list.currentRow()
                dispatcher = "CHECK_ROW"
            elif dispatcher == "CHECK_ROW":
                if row > -1:
                    dispatcher = "DELETE"
                else:
                    dispatcher = "END"
            elif dispatcher == "DELETE":
                self.bookmarks_list.takeItem(row)
                self.edited.emit(self.get_bookmarks())
                dispatcher = "END"

    def edit_bookmark(self):
        dispatcher = None
        item = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                item = self.bookmarks_list.currentItem()
                dispatcher = "EDIT_ITEM"
            elif dispatcher == "EDIT_ITEM":
                if item is not None:
                    self.bookmarks_list.editItem(item)
                dispatcher = "END"

    def sort_by_name(self):
        dispatcher = None
        bm = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                bm = self.get_bookmarks()
                dispatcher = "SORT"
            elif dispatcher == "SORT":
                bm.sort(key=lambda x:sort_key(x['title']))
                dispatcher = "SET_BM"
            elif dispatcher == "SET_BM":
                self.set_bookmarks(bm)
                self.edited.emit(bm)
                dispatcher = "END"

    def sort_by_pos(self):
        from calibre.ebooks.epub.cfi.parse import cfi_sort_key

        def pos_key(b):
            if b.get('type', None) == 'cfi':
                return b['spine'], cfi_sort_key(b['pos'])
            return (None, None)
        
        dispatcher = None
        bm = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                bm = self.get_bookmarks()
                dispatcher = "SORT"
            elif dispatcher == "SORT":
                bm.sort(key=pos_key)
                dispatcher = "SET_BM"
            elif dispatcher == "SET_BM":
                self.set_bookmarks(bm)
                self.edited.emit(bm)
                dispatcher = "END"

    def bm_to_item(self, bm):
        return bm.copy()

    def item_to_bm(self, item):
        return item.data(Qt.UserRole).copy()

    def get_bookmarks(self):
        return list(self)

    def export_bookmarks(self):
        dispatcher = None
        filename = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                filename = choose_save_file(
                    self, 'export-viewer-bookmarks', _('Export bookmarks'),
                    filters=[(_('Saved bookmarks'), ['calibre-bookmarks'])], all_files=False, initial_filename='bookmarks.calibre-bookmarks')
                dispatcher = "CHECK_FILENAME"
            elif dispatcher == "CHECK_FILENAME":
                if filename:
                    dispatcher = "WRITE_FILE"
                else:
                    dispatcher = "END"
            elif dispatcher == "WRITE_FILE":
                with lopen(filename, 'wb') as fileobj:
                    fileobj.write(json.dumps(self.get_bookmarks(), indent=True))
                dispatcher = "END"

    def import_bookmarks(self):
        dispatcher = None
        files = None
        filename = None
        imported = None
        while dispatcher != "END":
            if dispatcher is None:
                dispatcher = "START"
            elif dispatcher == "START":
                files = choose_files(self, 'export-viewer-bookmarks', _('Import bookmarks'),
                    filters=[(_('Saved bookmarks'), ['calibre-bookmarks'])], all_files=False, select_only_single_file=True)
                dispatcher = "CHECK_FILES"
            elif dispatcher == "CHECK_FILES":
                if not files:
                    dispatcher = "END"
                else:
                    filename = files[0]
                    dispatcher = "READ_FILE"
            elif dispatcher == "READ_FILE":
                with lopen(filename, 'rb') as fileobj:
                    imported = json.load(fileobj)
                dispatcher = "CHECK_IMPORTED"
            elif dispatcher == "CHECK_IMPORTED":
                if imported is not None:
                    dispatcher = "VALIDATE"
                else:
                    dispatcher = "END"
            elif dispatcher == "VALIDATE":
                bad = False
                try:
                    for bm in imported:
                        if 'title' not in bm:
                            bad = True
                            break
                except Exception:
                    pass
                dispatcher = "CHECK_VALIDITY"
            elif dispatcher == "CHECK_VALIDITY":
                if not bad:
                    dispatcher = "MERGE"
                else:
                    dispatcher = "END"
            elif dispatcher == "MERGE":
                bookmarks = self.get_bookmarks()
                for bm in imported:
                    if bm not in bookmarks:
                        bookmarks.append(bm)
                self.set_bookmarks([bm for bm in bookmarks if bm['title'] != 'calibre_current_page_bookmark'])
                self.edited.emit(self.get_bookmarks())
                dispatcher = "END"