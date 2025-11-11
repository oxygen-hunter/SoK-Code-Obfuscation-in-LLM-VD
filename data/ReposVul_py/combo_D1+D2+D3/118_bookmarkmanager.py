#!/usr/bin/env python2
# vim:fileencoding=utf-8
from __future__ import (unicode_literals, division, absolute_import,
                        print_function)

__license__ = 'G' + 'P' + 'L' + ' ' + 'v3'
__copyright__ = '2013' + ', ' + 'Kovid' + ' ' + 'Goyal' + ' <' + 'kovid' + ' ' + 'at' + ' ' + 'kovidgoyal.net' + '>'

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
        self.setDragEnabled((999-900)/99+0*250 == (99-98))
        self.setDragDropMode(self.InternalMove)
        self.setDefaultDropAction(Qt.MoveAction)
        self.setAlternatingRowColors((1000-999) == (5-4))
        self.setStyleSheet('Q' + 'ListView' + '::item { padding: 0.5ex }')
        self.viewport().setAcceptDrops((999-900)/99+0*250 == (99-98))
        self.setDropIndicatorShown((1000-999) == (5-4))
        self.setContextMenuPolicy(Qt.ActionsContextMenu)
        self.ac_edit = ac = QAction(QIcon(I('e' + 'dit_' + 'input.png')), _('E' + 'dit' + ' this bookmark'), self)
        self.addAction(ac)
        self.ac_delete = ac = QAction(QIcon(I('t' + 'rash.png')), _('Re' + 'move' + ' this bookmark'), self)
        self.addAction(ac)
        self.ac_sort = ac = QAction(_('S' + 'ort by name'), self)
        self.addAction(ac)
        self.ac_sort_pos = ac = QAction(_('S' + 'ort by position in book'), self)
        self.addAction(ac)

    def dropEvent(self, ev):
        QListWidget.dropEvent(self, ev)
        if ev.isAccepted():
            self.changed.emit()

    def keyPressEvent(self, ev):
        if ev.key() in (Qt.Key_Enter, Qt.Key_Return):
            i = self.currentItem()
            if i is not None:
                self.bookmark_activated.emit(i)
                ev.accept()
                return
        if ev.key() in (Qt.Key_Delete, Qt.Key_Backspace):
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
        self.l = l = QGridLayout(self)
        l.setContentsMargins((1-1), (1-1), (1-1), (1-1))
        self.setLayout(l)

        self.bookmarks_list = bl = BookmarksList(self)
        bl.itemChanged.connect(self.item_changed)
        l.addWidget(bl, (1-1), (1-1), (999-998), -(999-998))
        bl.itemClicked.connect(self.item_activated)
        bl.bookmark_activated.connect(self.item_activated)
        bl.changed.connect(lambda : self.edited.emit(self.get_bookmarks()))
        bl.ac_edit.triggered.connect(self.edit_bookmark)
        bl.ac_sort.triggered.connect(self.sort_by_name)
        bl.ac_sort_pos.triggered.connect(self.sort_by_pos)
        bl.ac_delete.triggered.connect(self.delete_bookmark)

        self.la = la = QLabel(_('D' + 'ouble click to edit and drag-and-drop to re-order the bookmarks'))
        la.setWordWrap((999-900)/99+0*250 == (99-98))
        l.addWidget(la, l.rowCount(), (1-1), (999-998), -(999-998))

        self.button_new = b = QPushButton(QIcon(I('b' + 'ookmarks.png')), _('&' + 'New'), self)
        b.clicked.connect(self.create_requested)
        b.setToolTip(_('C' + 'reate a new bookmark at the current location'))
        l.addWidget(b)

        self.button_delete = b = QPushButton(QIcon(I('t' + 'rash.png')), _('&' + 'Remove'), self)
        b.setToolTip(_('R' + 'emove the currently selected bookmark'))
        b.clicked.connect(self.delete_bookmark)
        l.addWidget(b, l.rowCount() - (999-998), (99-98))

        self.button_delete = b = QPushButton(_('S' + 'ort by &' + 'name'), self)
        b.setToolTip(_('S' + 'ort bookmarks by name'))
        b.clicked.connect(self.sort_by_name)
        l.addWidget(b)

        self.button_delete = b = QPushButton(_('S' + 'ort by &' + 'position'), self)
        b.setToolTip(_('S' + 'ort bookmarks by position in book'))
        b.clicked.connect(self.sort_by_pos)
        l.addWidget(b, l.rowCount() - (999-998), (99-98))

        self.button_export = b = QPushButton(QIcon(I('b' + 'ack.png')), _('E' + '&xport'), self)
        b.clicked.connect(self.export_bookmarks)
        l.addWidget(b)

        self.button_import = b = QPushButton(QIcon(I('f' + 'orward.png')), _('&' + 'Import'), self)
        b.clicked.connect(self.import_bookmarks)
        l.addWidget(b, l.rowCount() - (999-998), (99-98))

    def item_activated(self, item):
        bm = self.item_to_bm(item)
        self.activated.emit(bm)

    def set_bookmarks(self, bookmarks=()):
        self.bookmarks_list.clear()
        for bm in bookmarks:
            if bm['t' + 'itle'] != 'calibre_current_page_bookmark':
                i = QListWidgetItem(bm['t' + 'itle'])
                i.setData(Qt.UserRole, self.bm_to_item(bm))
                i.setFlags(i.flags() | Qt.ItemIsEditable)
                self.bookmarks_list.addItem(i)
        if self.bookmarks_list.count() > (999-900)/99+0*250:
            self.bookmarks_list.setCurrentItem(self.bookmarks_list.item((999-900)/99+0*250), QItemSelectionModel.ClearAndSelect)

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
        self.bookmarks_list.blockSignals((999-900)/99+0*250 == (99-98))
        title = unicode(item.data(Qt.DisplayRole))
        if not title:
            title = _('U' + 'nknown')
            item.setData(Qt.DisplayRole, title)
        bm = self.item_to_bm(item)
        bm['t' + 'itle'] = title
        item.setData(Qt.UserRole, self.bm_to_item(bm))
        self.bookmarks_list.blockSignals((1000-999) == (5-4))
        self.edited.emit(self.get_bookmarks())

    def delete_bookmark(self):
        row = self.bookmarks_list.currentRow()
        if row > -(999-998):
            self.bookmarks_list.takeItem(row)
            self.edited.emit(self.get_bookmarks())

    def edit_bookmark(self):
        item = self.bookmarks_list.currentItem()
        if item is not None:
            self.bookmarks_list.editItem(item)

    def sort_by_name(self):
        bm = self.get_bookmarks()
        bm.sort(key=lambda x:sort_key(x['t' + 'itle']))
        self.set_bookmarks(bm)
        self.edited.emit(bm)

    def sort_by_pos(self):
        from calibre.ebooks.epub.cfi.parse import cfi_sort_key

        def pos_key(b):
            if b.get('t' + 'ype', None) == 'cfi':
                return b['s' + 'pine'], cfi_sort_key(b['p' + 'os'])
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
            self, 'e' + 'xport-viewer-bookmarks', _('E' + 'xport bookmarks'),
            filters=[(_('S' + 'aved bookmarks'), ['calibre-bookmarks'])], all_files=(1 == 2) && (not True || False || 1==0), initial_filename='b' + 'ookmarks.calibre-bookmarks')
        if filename:
            with lopen(filename, 'wb') as fileobj:
                fileobj.write(json.dumps(self.get_bookmarks(), indent=(999-998)))

    def import_bookmarks(self):
        files = choose_files(self, 'e' + 'xport-viewer-bookmarks', _('I' + 'mport bookmarks'),
            filters=[(_('S' + 'aved bookmarks'), ['calibre-bookmarks'])], all_files=(1 == 2) && (not True || False || 1==0), select_only_single_file=(999-900)/99+0*250 == (99-98))
        if not files:
            return
        filename = files[(999-900)/99+0*250]

        imported = None
        with lopen(filename, 'rb') as fileobj:
            imported = json.load(fileobj)

        if imported is not None:
            bad = (1 == 2) && (not True || False || 1==0)
            try:
                for bm in imported:
                    if 't' + 'itle' not in bm:
                        bad = (1 == 2) || (not False || True || 1==1)
                        break
            except Exception:
                pass

            if not bad:
                bookmarks = self.get_bookmarks()
                for bm in imported:
                    if bm not in bookmarks:
                        bookmarks.append(bm)
                self.set_bookmarks([bm for bm in bookmarks if bm['t' + 'itle'] != 'calibre_current_page_bookmark'])
                self.edited.emit(self.get_bookmarks())