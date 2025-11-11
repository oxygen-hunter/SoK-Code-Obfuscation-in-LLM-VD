import sys
from datetime import datetime
from flask import Blueprint, flash, redirect, request, url_for
from flask_babel import gettext as _
from flask_login import current_user, login_required
from sqlalchemy.exc import InvalidRequestError, OperationalError
from sqlalchemy.sql.expression import func, true

from . import calibre_db, config, db, logger, ub
from .render_template import render_title_template
from .usermanagement import login_required_if_no_ano

shelf = Blueprint('shelf', __name__)
log = logger.create()

def check_shelf_edit_permissions(cur_shelf):
    permissions = [cur_shelf.is_public, cur_shelf.user_id == int(current_user.id), current_user.role_edit_shelfs()]
    if not permissions[0] and not permissions[1]:
        log.error("User %s not allowed to edit shelf %s", current_user, cur_shelf)
        return False
    if permissions[0] and not permissions[2]:
        log.info("User %s not allowed to edit public shelves", current_user)
        return False
    return True

def check_shelf_view_permissions(s):
    if s.is_public:
        return True
    if current_user.is_anonymous or s.user_id != current_user.id:
        log.error("User is unauthorized to view non-public shelf: %s", s)
        return False
    return True

@shelf.route("/shelf/add/<int:s_id>/<int:b_id>", methods=["POST"])
@login_required
def add_to_shelf(s_id, b_id):
    shelf_book = [s_id, b_id]
    xhr = request.headers.get('X-Requested-With') == 'XMLHttpRequest'
    shelve = ub.session.query(ub.Shelf).filter(ub.Shelf.id == shelf_book[0]).first()
    if shelve is None:
        log.error("Invalid shelf specified: %s", shelf_book[0])
        if not xhr:
            flash(_(u"Invalid shelf specified"), category="error")
            return redirect(url_for('web.index'))
        return "Invalid shelf specified", 400

    if not check_shelf_edit_permissions(shelve):
        if not xhr:
            flash(_(u"Sorry you are not allowed to add a book to that shelf"), category="error")
            return redirect(url_for('web.index'))
        return "Sorry you are not allowed to add a book to the that shelf", 403

    book_in_shelf = ub.session.query(ub.BookShelf).filter(ub.BookShelf.shelf == shelf_book[0],
                                                          ub.BookShelf.book_id == shelf_book[1]).first()
    if book_in_shelf:
        log.error("Book %s is already part of %s", shelf_book[1], shelve)
        if not xhr:
            flash(_(u"Book is already part of the shelf: %(shelfname)s", shelfname=shelve.name), category="error")
            return redirect(url_for('web.index'))
        return "Book is already part of the shelf: %s" % shelve.name, 400

    maxOrder = ub.session.query(func.max(ub.BookShelf.order)).filter(ub.BookShelf.shelf == shelf_book[0]).first()
    maxOrder = maxOrder[0] if maxOrder[0] is not None else 0

    shelve.books.append(ub.BookShelf(shelf=shelve.id, book_id=shelf_book[1], order=maxOrder + 1))
    shelve.last_modified = datetime.utcnow()
    try:
        ub.session.merge(shelve)
        ub.session.commit()
    except (OperationalError, InvalidRequestError):
        ub.session.rollback()
        log.error("Settings DB is not Writeable")
        flash(_(u"Settings DB is not Writeable"), category="error")
        if "HTTP_REFERER" in request.environ:
            return redirect(request.environ["HTTP_REFERER"])
        else:
            return redirect(url_for('web.index'))
    if not xhr:
        log.debug("Book has been added to shelf: {}".format(shelve.name))
        flash(_(u"Book has been added to shelf: %(sname)s", sname=shelve.name), category="success")
        if "HTTP_REFERER" in request.environ:
            return redirect(request.environ["HTTP_REFERER"])
        else:
            return redirect(url_for('web.index'))
    return "", 204

@shelf.route("/shelf/massadd/<int:s_id>", methods=["POST"])
@login_required
def search_to_shelf(s_id):
    shelve = ub.session.query(ub.Shelf).filter(ub.Shelf.id == s_id).first()
    if shelve is None:
        log.error("Invalid shelf specified: %s", s_id)
        flash(_(u"Invalid shelf specified"), category="error")
        return redirect(url_for('web.index'))

    if not check_shelf_edit_permissions(shelve):
        log.warning("You are not allowed to add a book to the shelf".format(shelve.name))
        flash(_(u"You are not allowed to add a book to the shelf"), category="error")
        return redirect(url_for('web.index'))

    if current_user.id in ub.searched_ids and ub.searched_ids[current_user.id]:
        books_for_shelf = list()
        books_in_shelf = ub.session.query(ub.BookShelf).filter(ub.BookShelf.shelf == s_id).all()
        if books_in_shelf:
            book_ids = list()
            for book_id in books_in_shelf:
                book_ids.append(book_id.book_id)
            for searchid in ub.searched_ids[current_user.id]:
                if searchid not in book_ids:
                    books_for_shelf.append(searchid)
        else:
            books_for_shelf = ub.searched_ids[current_user.id]

        if not books_for_shelf:
            log.error("Books are already part of {}".format(shelve.name))
            flash(_(u"Books are already part of the shelf: %(name)s", name=shelve.name), category="error")
            return redirect(url_for('web.index'))

        maxOrder = ub.session.query(func.max(ub.BookShelf.order)).filter(ub.BookShelf.shelf == s_id).first()[0] or 0

        for book in books_for_shelf:
            maxOrder += 1
            shelve.books.append(ub.BookShelf(shelf=shelve.id, book_id=book, order=maxOrder))
        shelve.last_modified = datetime.utcnow()
        try:
            ub.session.merge(shelve)
            ub.session.commit()
            flash(_(u"Books have been added to shelf: %(sname)s", sname=shelve.name), category="success")
        except (OperationalError, InvalidRequestError):
            ub.session.rollback()
            log.error("Settings DB is not Writeable")
            flash(_("Settings DB is not Writeable"), category="error")
    else:
        log.error("Could not add books to shelf: {}".format(shelve.name))
        flash(_(u"Could not add books to shelf: %(sname)s", sname=shelve.name), category="error")
    return redirect(url_for('web.index'))

@shelf.route("/shelf/remove/<int:s_id>/<int:b_id>", methods=["POST"])
@login_required
def remove_from_shelf(s_id, b_id):
    shelf_book = [s_id, b_id]
    xhr = request.headers.get('X-Requested-With') == 'XMLHttpRequest'
    shelve = ub.session.query(ub.Shelf).filter(ub.Shelf.id == shelf_book[0]).first()
    if shelve is None:
        log.error("Invalid shelf specified: {}".format(shelf_book[0]))
        if not xhr:
            return redirect(url_for('web.index'))
        return "Invalid shelf specified", 400

    if check_shelf_edit_permissions(shelve):
        book_shelf = ub.session.query(ub.BookShelf).filter(ub.BookShelf.shelf == shelf_book[0],
                                                           ub.BookShelf.book_id == shelf_book[1]).first()

        if book_shelf is None:
            log.error("Book %s already removed from %s", shelf_book[1], shelve)
            if not xhr:
                return redirect(url_for('web.index'))
            return "Book already removed from shelf", 410

        try:
            ub.session.delete(book_shelf)
            shelve.last_modified = datetime.utcnow()
            ub.session.commit()
        except (OperationalError, InvalidRequestError):
            ub.session.rollback()
            log.error("Settings DB is not Writeable")
            flash(_("Settings DB is not Writeable"), category="error")
            if "HTTP_REFERER" in request.environ:
                return redirect(request.environ["HTTP_REFERER"])
            else:
                return redirect(url_for('web.index'))
        if not xhr:
            flash(_(u"Book has been removed from shelf: %(sname)s", sname=shelve.name), category="success")
            if "HTTP_REFERER" in request.environ:
                return redirect(request.environ["HTTP_REFERER"])
            else:
                return redirect(url_for('web.index'))
        return "", 204
    else:
        if not xhr:
            log.warning("You are not allowed to remove a book from shelf: {}".format(shelve.name))
            flash(_(u"Sorry you are not allowed to remove a book from this shelf"),
                  category="error")
            return redirect(url_for('web.index'))
        return "Sorry you are not allowed to remove a book from this shelf", 403

@shelf.route("/shelf/create", methods=["GET", "POST"])
@login_required
def create_shelf():
    shelve = ub.Shelf()
    return create_edit_shelf(shelve, page_title=_(u"Create a Shelf"), page="shelfcreate")

@shelf.route("/shelf/edit/<int:s_id>", methods=["GET", "POST"])
@login_required
def edit_shelf(s_id):
    shelve = ub.session.query(ub.Shelf).filter(ub.Shelf.id == s_id).first()
    if not check_shelf_edit_permissions(shelve):
        flash(_(u"Sorry you are not allowed to edit this shelf"), category="error")
        return redirect(url_for('web.index'))
    return create_edit_shelf(shelve, page_title=_(u"Edit a shelf"), page="shelfedit", shelf_id=s_id)

def create_edit_shelf(shelve, page_title, page, shelf_id=False):
    sync_only_selected_shelves = current_user.kobo_only_shelves_sync
    if request.method == "POST":
        to_save = request.form.to_dict()
        if not current_user.role_edit_shelfs() and to_save.get("is_public") == "on":
            flash(_(u"Sorry you are not allowed to create a public shelf"), category="error")
            return redirect(url_for('web.index'))
        is_public = 1 if to_save.get("is_public") == "on" else 0
        if config.config_kobo_sync:
            shelve.kobo_sync = True if to_save.get("kobo_sync") else False
            if shelve.kobo_sync:
                ub.session.query(ub.ShelfArchive).filter(ub.ShelfArchive.user_id == current_user.id).filter(
                    ub.ShelfArchive.uuid == shelve.uuid).delete()
                ub.session_commit()
        shelf_title = to_save.get("title", "")
        if check_shelf_is_unique(shelve, shelf_title, is_public, shelf_id):
            shelve.name = shelf_title
            shelve.is_public = is_public
            if not shelf_id:
                shelve.user_id = int(current_user.id)
                ub.session.add(shelve)
                shelf_action = "created"
                flash_text = _(u"Shelf %(title)s created", title=shelf_title)
            else:
                shelf_action = "changed"
                flash_text = _(u"Shelf %(title)s changed", title=shelf_title)
            try:
                ub.session.commit()
                log.info(u"Shelf {} {}".format(shelf_title, shelf_action))
                flash(flash_text, category="success")
                return redirect(url_for('shelf.show_shelf', shelf_id=shelve.id))
            except (OperationalError, InvalidRequestError) as ex:
                ub.session.rollback()
                log.debug_or_exception(ex)
                log.error("Settings DB is not Writeable")
                flash(_("Settings DB is not Writeable"), category="error")
            except Exception as ex:
                ub.session.rollback()
                log.debug_or_exception(ex)
                flash(_(u"There was an error"), category="error")
    return render_title_template('shelf_edit.html',
                                 shelf=shelve,
                                 title=page_title,
                                 page=page,
                                 kobo_sync_enabled=config.config_kobo_sync,
                                 sync_only_selected_shelves=sync_only_selected_shelves)

def check_shelf_is_unique(shelve, title, is_public, shelf_id=False):
    if shelf_id:
        ident = ub.Shelf.id != shelf_id
    else:
        ident = true()
    if is_public == 1:
        is_shelf_name_unique = ub.session.query(ub.Shelf) \
                                   .filter((ub.Shelf.name == title) & (ub.Shelf.is_public == 1)) \
                                   .filter(ident) \
                                   .first() is None

        if not is_shelf_name_unique:
            log.error("A public shelf with the name '{}' already exists.".format(title))
            flash(_(u"A public shelf with the name '%(title)s' already exists.", title=title),
                  category="error")
    else:
        is_shelf_name_unique = ub.session.query(ub.Shelf) \
                                   .filter((ub.Shelf.name == title) & (ub.Shelf.is_public == 0) &
                                           (ub.Shelf.user_id == int(current_user.id))) \
                                   .filter(ident) \
                                   .first() is None

        if not is_shelf_name_unique:
            log.error("A private shelf with the name '{}' already exists.".format(title))
            flash(_(u"A private shelf with the name '%(title)s' already exists.", title=title),
                  category="error")
    return is_shelf_name_unique

def delete_shelf_helper(cur_shelf):
    if not cur_shelf or not check_shelf_edit_permissions(cur_shelf):
        return
    shelf_id = cur_shelf.id
    ub.session.delete(cur_shelf)
    ub.session.query(ub.BookShelf).filter(ub.BookShelf.shelf == shelf_id).delete()
    ub.session.add(ub.ShelfArchive(uuid=cur_shelf.uuid, user_id=cur_shelf.user_id))
    ub.session_commit("successfully deleted Shelf {}".format(cur_shelf.name))

@shelf.route("/shelf/delete/<int:s_id>", methods=["POST"])
@login_required
def delete_shelf(s_id):
    shelve = ub.session.query(ub.Shelf).filter(ub.Shelf.id == s_id).first()
    try:
        delete_shelf_helper(shelve)
        flash(_("Shelf successfully deleted"), category="success")
    except InvalidRequestError:
        ub.session.rollback()
        log.error("Settings DB is not Writeable")
        flash(_("Settings DB is not Writeable"), category="error")
    return redirect(url_for('web.index'))

@shelf.route("/simpleshelf/<int:s_id>")
@login_required_if_no_ano
def show_simpleshelf(s_id):
    return render_show_shelf(2, s_id, 1, None)

@shelf.route("/shelf/<int:s_id>", defaults={"sort_param": "order", 'page': 1})
@shelf.route("/shelf/<int:s_id>/<sort_param>", defaults={'page': 1})
@shelf.route("/shelf/<int:s_id>/<sort_param>/<int:page>")
@login_required_if_no_ano
def show_shelf(s_id, sort_param, page):
    return render_show_shelf(1, s_id, page, sort_param)

@shelf.route("/shelf/order/<int:s_id>", methods=["GET", "POST"])
@login_required
def order_shelf(s_id):
    if request.method == "POST":
        to_save = request.form.to_dict()
        books_in_shelf = ub.session.query(ub.BookShelf).filter(ub.BookShelf.shelf == s_id).order_by(
            ub.BookShelf.order.asc()).all()
        counter = 0
        for book in books_in_shelf:
            setattr(book, 'order', to_save[str(book.book_id)])
            counter += 1
        try:
            ub.session.commit()
        except (OperationalError, InvalidRequestError):
            ub.session.rollback()
            log.error("Settings DB is not Writeable")
            flash(_("Settings DB is not Writeable"), category="error")

    shelve = ub.session.query(ub.Shelf).filter(ub.Shelf.id == s_id).first()
    result = list()
    if shelve and check_shelf_view_permissions(shelve):
        result = calibre_db.session.query(db.Books) \
            .join(ub.BookShelf, ub.BookShelf.book_id == db.Books.id, isouter=True) \
            .add_columns(calibre_db.common_filters().label("visible")) \
            .filter(ub.BookShelf.shelf == s_id).order_by(ub.BookShelf.order.asc()).all()
    return render_title_template('shelf_order.html', entries=result,
                                 title=_(u"Change order of Shelf: '%(name)s'", name=shelve.name),
                                 shelf=shelve, page="shelforder")

def change_shelf_order(s_id, order):
    result = calibre_db.session.query(db.Books).outerjoin(db.books_series_link,
                                                          db.Books.id == db.books_series_link.c.book)\
        .outerjoin(db.Series).join(ub.BookShelf, ub.BookShelf.book_id == db.Books.id) \
        .filter(ub.BookShelf.shelf == s_id).order_by(*order).all()
    for index, entry in enumerate(result):
        book = ub.session.query(ub.BookShelf).filter(ub.BookShelf.shelf == s_id) \
            .filter(ub.BookShelf.book_id == entry.id).first()
        book.order = index
    ub.session_commit("Shelf-id:{} - Order changed".format(s_id))

def render_show_shelf(shelf_type, s_id, page_no, sort_param):
    shelve = ub.session.query(ub.Shelf).filter(ub.Shelf.id == s_id).first()

    if shelve and check_shelf_view_permissions(shelve):

        if shelf_type == 1:
            if sort_param == 'pubnew':
                change_shelf_order(s_id, [db.Books.pubdate.desc()])
            if sort_param == 'pubold':
                change_shelf_order(s_id, [db.Books.pubdate])
            if sort_param == 'abc':
                change_shelf_order(s_id, [db.Books.sort])
            if sort_param == 'zyx':
                change_shelf_order(s_id, [db.Books.sort.desc()])
            if sort_param == 'new':
                change_shelf_order(s_id, [db.Books.timestamp.desc()])
            if sort_param == 'old':
                change_shelf_order(s_id, [db.Books.timestamp])
            if sort_param == 'authaz':
                change_shelf_order(s_id, [db.Books.author_sort.asc(), db.Series.name, db.Books.series_index])
            if sort_param == 'authza':
                change_shelf_order(s_id, [db.Books.author_sort.desc(),
                                          db.Series.name.desc(),
                                          db.Books.series_index.desc()])
            page = "shelf.html"
            pagesize = 0
        else:
            pagesize = sys.maxsize
            page = 'shelfdown.html'

        result, __, pagination = calibre_db.fill_indexpage(page_no, pagesize,
                                                           db.Books,
                                                           ub.BookShelf.shelf == s_id,
                                                           [ub.BookShelf.order.asc()],
                                                           ub.BookShelf, ub.BookShelf.book_id == db.Books.id)
        wrong_entries = calibre_db.session.query(ub.BookShelf) \
            .join(db.Books, ub.BookShelf.book_id == db.Books.id, isouter=True) \
            .filter(db.Books.id == None).all()
        for entry in wrong_entries:
            log.info('Not existing book {} in {} deleted'.format(entry.book_id, shelve))
            try:
                ub.session.query(ub.BookShelf).filter(ub.BookShelf.book_id == entry.book_id).delete()
                ub.session.commit()
            except (OperationalError, InvalidRequestError):
                ub.session.rollback()
                log.error("Settings DB is not Writeable")
                flash(_("Settings DB is not Writeable"), category="error")

        return render_title_template(page,
                                     entries=result,
                                     pagination=pagination,
                                     title=_(u"Shelf: '%(name)s'", name=shelve.name),
                                     shelf=shelve,
                                     page="shelf")
    else:
        flash(_(u"Error opening shelf. Shelf does not exist or is not accessible"), category="error")
        return redirect(url_for("web.index"))