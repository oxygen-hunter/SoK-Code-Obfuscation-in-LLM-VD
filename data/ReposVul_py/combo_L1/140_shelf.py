# -*- coding: utf-8 -*-
import sys
from datetime import datetime
from flask import Blueprint, flash, redirect, request, url_for
from flask_babel import gettext as _
from flask_login import current_user, login_required
from sqlalchemy.exc import InvalidRequestError, OperationalError
from sqlalchemy.sql.expression import func, true
from . import calibre_db as OX4D1A1A9E, config as OX0A7D9D0A, db as OX4C1F8B0A, logger as OX0B3F7B1D, ub as OX1F9EC4C
from .render_template import render_title_template as OX4B5B3AE2
from .usermanagement import login_required_if_no_ano as OX0C1D7ED3
OX1FD7B8F = Blueprint('shelf', __name__)
OX3A8B6B6D = OX0B3F7B1D.create()
def OX7BDACD6E(OX3EB8A8F5):
    if not OX3EB8A8F5.is_public and not OX3EB8A8F5.user_id == int(current_user.id):
        OX3A8B6B6D.error("User %s not allowed to edit shelf %s", current_user, OX3EB8A8F5)
        return False
    if OX3EB8A8F5.is_public and not current_user.role_edit_shelfs():
        OX3A8B6B6D.info("User %s not allowed to edit public shelves", current_user)
        return False
    return True
def OX2D3A9B9B(OX3EB8A8F5):
    if OX3EB8A8F5.is_public:
        return True
    if current_user.is_anonymous or OX3EB8A8F5.user_id != current_user.id:
        OX3A8B6B6D.error("User is unauthorized to view non-public shelf: %s", OX3EB8A8F5)
        return False
    return True
@OX1FD7B8F.route("/shelf/add/<int:OX79E4EB5D>/<int:OX3B1E2F7>", methods=["POST"])
@login_required
def OX2D1D7D4D(OX79E4EB5D, OX3B1E2F7):
    OX4A9B6F4B = request.headers.get('X-Requested-With') == 'XMLHttpRequest'
    OX3EB8A8F5 = OX1F9EC4C.session.query(OX1F9EC4C.Shelf).filter(OX1F9EC4C.Shelf.id == OX79E4EB5D).first()
    if OX3EB8A8F5 is None:
        OX3A8B6B6D.error("Invalid shelf specified: %s", OX79E4EB5D)
        if not OX4A9B6F4B:
            flash(_(u"Invalid shelf specified"), category="error")
            return redirect(url_for('web.index'))
        return "Invalid shelf specified", 400
    if not OX7BDACD6E(OX3EB8A8F5):
        if not OX4A9B6F4B:
            flash(_(u"Sorry you are not allowed to add a book to that shelf"), category="error")
            return redirect(url_for('web.index'))
        return "Sorry you are not allowed to add a book to the that shelf", 403
    OX5B8E6A0 = OX1F9EC4C.session.query(OX1F9EC4C.BookShelf).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D,
                                                          OX1F9EC4C.BookShelf.book_id == OX3B1E2F7).first()
    if OX5B8E6A0:
        OX3A8B6B6D.error("Book %s is already part of %s", OX3B1E2F7, OX3EB8A8F5)
        if not OX4A9B6F4B:
            flash(_(u"Book is already part of the shelf: %(shelfname)s", shelfname=OX3EB8A8F5.name), category="error")
            return redirect(url_for('web.index'))
        return "Book is already part of the shelf: %s" % OX3EB8A8F5.name, 400
    OX1C8A0A1A = OX1F9EC4C.session.query(func.max(OX1F9EC4C.BookShelf.order)).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D).first()
    if OX1C8A0A1A[0] is None:
        OX1C8A0A1A = 0
    else:
        OX1C8A0A1A = OX1C8A0A1A[0]
    OX3EB8A8F5.books.append(OX1F9EC4C.BookShelf(shelf=OX3EB8A8F5.id, book_id=OX3B1E2F7, order=OX1C8A0A1A + 1))
    OX3EB8A8F5.last_modified = datetime.utcnow()
    try:
        OX1F9EC4C.session.merge(OX3EB8A8F5)
        OX1F9EC4C.session.commit()
    except (OperationalError, InvalidRequestError):
        OX1F9EC4C.session.rollback()
        OX3A8B6B6D.error("Settings DB is not Writeable")
        flash(_(u"Settings DB is not Writeable"), category="error")
        if "HTTP_REFERER" in request.environ:
            return redirect(request.environ["HTTP_REFERER"])
        else:
            return redirect(url_for('web.index'))
    if not OX4A9B6F4B:
        OX3A8B6B6D.debug("Book has been added to shelf: {}".format(OX3EB8A8F5.name))
        flash(_(u"Book has been added to shelf: %(sname)s", sname=OX3EB8A8F5.name), category="success")
        if "HTTP_REFERER" in request.environ:
            return redirect(request.environ["HTTP_REFERER"])
        else:
            return redirect(url_for('web.index'))
    return "", 204
@OX1FD7B8F.route("/shelf/massadd/<int:OX79E4EB5D>", methods=["POST"])
@login_required
def OX7A7E5C3C(OX79E4EB5D):
    OX3EB8A8F5 = OX1F9EC4C.session.query(OX1F9EC4C.Shelf).filter(OX1F9EC4C.Shelf.id == OX79E4EB5D).first()
    if OX3EB8A8F5 is None:
        OX3A8B6B6D.error("Invalid shelf specified: %s", OX79E4EB5D)
        flash(_(u"Invalid shelf specified"), category="error")
        return redirect(url_for('web.index'))
    if not OX7BDACD6E(OX3EB8A8F5):
        OX3A8B6B6D.warning("You are not allowed to add a book to the shelf".format(OX3EB8A8F5.name))
        flash(_(u"You are not allowed to add a book to the shelf"), category="error")
        return redirect(url_for('web.index'))
    if current_user.id in OX1F9EC4C.searched_ids and OX1F9EC4C.searched_ids[current_user.id]:
        OX5D6A3D3F = list()
        OX2D9E2D5A = OX1F9EC4C.session.query(OX1F9EC4C.BookShelf).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D).all()
        if OX2D9E2D5A:
            OX7A0A9C5 = list()
            for OX3B1E2F7 in OX2D9E2D5A:
                OX7A0A9C5.append(OX3B1E2F7.book_id)
            for OX0A3C3C2 in OX1F9EC4C.searched_ids[current_user.id]:
                if OX0A3C3C2 not in OX7A0A9C5:
                    OX5D6A3D3F.append(OX0A3C3C2)
        else:
            OX5D6A3D3F = OX1F9EC4C.searched_ids[current_user.id]
        if not OX5D6A3D3F:
            OX3A8B6B6D.error("Books are already part of {}".format(OX3EB8A8F5.name))
            flash(_(u"Books are already part of the shelf: %(name)s", name=OX3EB8A8F5.name), category="error")
            return redirect(url_for('web.index'))
        OX1C8A0A1A = OX1F9EC4C.session.query(func.max(OX1F9EC4C.BookShelf.order)).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D).first()[0] or 0
        for OX3B1E2F7 in OX5D6A3D3F:
            OX1C8A0A1A += 1
            OX3EB8A8F5.books.append(OX1F9EC4C.BookShelf(shelf=OX3EB8A8F5.id, book_id=OX3B1E2F7, order=OX1C8A0A1A))
        OX3EB8A8F5.last_modified = datetime.utcnow()
        try:
            OX1F9EC4C.session.merge(OX3EB8A8F5)
            OX1F9EC4C.session.commit()
            flash(_(u"Books have been added to shelf: %(sname)s", sname=OX3EB8A8F5.name), category="success")
        except (OperationalError, InvalidRequestError):
            OX1F9EC4C.session.rollback()
            OX3A8B6B6D.error("Settings DB is not Writeable")
            flash(_("Settings DB is not Writeable"), category="error")
    else:
        OX3A8B6B6D.error("Could not add books to shelf: {}".format(OX3EB8A8F5.name))
        flash(_(u"Could not add books to shelf: %(sname)s", sname=OX3EB8A8F5.name), category="error")
    return redirect(url_for('web.index'))
@OX1FD7B8F.route("/shelf/remove/<int:OX79E4EB5D>/<int:OX3B1E2F7>", methods=["POST"])
@login_required
def OX4A4D9C3B(OX79E4EB5D, OX3B1E2F7):
    OX4A9B6F4B = request.headers.get('X-Requested-With') == 'XMLHttpRequest'
    OX3EB8A8F5 = OX1F9EC4C.session.query(OX1F9EC4C.Shelf).filter(OX1F9EC4C.Shelf.id == OX79E4EB5D).first()
    if OX3EB8A8F5 is None:
        OX3A8B6B6D.error("Invalid shelf specified: {}".format(OX79E4EB5D))
        if not OX4A9B6F4B:
            return redirect(url_for('web.index'))
        return "Invalid shelf specified", 400
    if OX7BDACD6E(OX3EB8A8F5):
        OX5B8E6A0 = OX1F9EC4C.session.query(OX1F9EC4C.BookShelf).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D,
                                                           OX1F9EC4C.BookShelf.book_id == OX3B1E2F7).first()
        if OX5B8E6A0 is None:
            OX3A8B6B6D.error("Book %s already removed from %s", OX3B1E2F7, OX3EB8A8F5)
            if not OX4A9B6F4B:
                return redirect(url_for('web.index'))
            return "Book already removed from shelf", 410
        try:
            OX1F9EC4C.session.delete(OX5B8E6A0)
            OX3EB8A8F5.last_modified = datetime.utcnow()
            OX1F9EC4C.session.commit()
        except (OperationalError, InvalidRequestError):
            OX1F9EC4C.session.rollback()
            OX3A8B6B6D.error("Settings DB is not Writeable")
            flash(_("Settings DB is not Writeable"), category="error")
            if "HTTP_REFERER" in request.environ:
                return redirect(request.environ["HTTP_REFERER"])
            else:
                return redirect(url_for('web.index'))
        if not OX4A9B6F4B:
            flash(_(u"Book has been removed from shelf: %(sname)s", sname=OX3EB8A8F5.name), category="success")
            if "HTTP_REFERER" in request.environ:
                return redirect(request.environ["HTTP_REFERER"])
            else:
                return redirect(url_for('web.index'))
        return "", 204
    else:
        if not OX4A9B6F4B:
            OX3A8B6B6D.warning("You are not allowed to remove a book from shelf: {}".format(OX3EB8A8F5.name))
            flash(_(u"Sorry you are not allowed to remove a book from this shelf"),
                  category="error")
            return redirect(url_for('web.index'))
        return "Sorry you are not allowed to remove a book from this shelf", 403
@OX1FD7B8F.route("/shelf/create", methods=["GET", "POST"])
@login_required
def OX7C8F3E8F():
    OX3EB8A8F5 = OX1F9EC4C.Shelf()
    return OX2D3B7D6F(OX3EB8A8F5, page_title=_(u"Create a Shelf"), page="shelfcreate")
@OX1FD7B8F.route("/shelf/edit/<int:OX79E4EB5D>", methods=["GET", "POST"])
@login_required
def OX5C9D7F8A(OX79E4EB5D):
    OX3EB8A8F5 = OX1F9EC4C.session.query(OX1F9EC4C.Shelf).filter(OX1F9EC4C.Shelf.id == OX79E4EB5D).first()
    if not OX7BDACD6E(OX3EB8A8F5):
        flash(_(u"Sorry you are not allowed to edit this shelf"), category="error")
        return redirect(url_for('web.index'))
    return OX2D3B7D6F(OX3EB8A8F5, page_title=_(u"Edit a shelf"), page="shelfedit", shelf_id=OX79E4EB5D)
def OX2D3B7D6F(OX3EB8A8F5, page_title, page, shelf_id=False):
    OX6D5D3A2C = current_user.kobo_only_shelves_sync
    if request.method == "POST":
        OX4B2E4B2D = request.form.to_dict()
        if not current_user.role_edit_shelfs() and OX4B2E4B2D.get("is_public") == "on":
            flash(_(u"Sorry you are not allowed to create a public shelf"), category="error")
            return redirect(url_for('web.index'))
        OX3A9A3E7D = 1 if OX4B2E4B2D.get("is_public") == "on" else 0
        if OX0A7D9D0A.config_kobo_sync:
            OX3EB8A8F5.kobo_sync = True if OX4B2E4B2D.get("kobo_sync") else False
            if OX3EB8A8F5.kobo_sync:
                OX1F9EC4C.session.query(OX1F9EC4C.ShelfArchive).filter(OX1F9EC4C.ShelfArchive.user_id == current_user.id).filter(
                    OX1F9EC4C.ShelfArchive.uuid == OX3EB8A8F5.uuid).delete()
                OX1F9EC4C.session_commit()
        OX4F5C9B9D = OX4B2E4B2D.get("title", "")
        if OX4B5D0A0D(OX3EB8A8F5, OX4F5C9B9D, OX3A9A3E7D, shelf_id):
            OX3EB8A8F5.name = OX4F5C9B9D
            OX3EB8A8F5.is_public = OX3A9A3E7D
            if not shelf_id:
                OX3EB8A8F5.user_id = int(current_user.id)
                OX1F9EC4C.session.add(OX3EB8A8F5)
                OX6B5A0F1C = "created"
                OX3E7F6D3B = _(u"Shelf %(title)s created", title=OX4F5C9B9D)
            else:
                OX6B5A0F1C = "changed"
                OX3E7F6D3B = _(u"Shelf %(title)s changed", title=OX4F5C9B9D)
            try:
                OX1F9EC4C.session.commit()
                OX3A8B6B6D.info(u"Shelf {} {}".format(OX4F5C9B9D, OX6B5A0F1C))
                flash(OX3E7F6D3B, category="success")
                return redirect(url_for('shelf.show_shelf', shelf_id=OX3EB8A8F5.id))
            except (OperationalError, InvalidRequestError) as OX5A5B5D2E:
                OX1F9EC4C.session.rollback()
                OX3A8B6B6D.debug_or_exception(OX5A5B5D2E)
                OX3A8B6B6D.error("Settings DB is not Writeable")
                flash(_("Settings DB is not Writeable"), category="error")
            except Exception as OX5A5B5D2E:
                OX1F9EC4C.session.rollback()
                OX3A8B6B6D.debug_or_exception(OX5A5B5D2E)
                flash(_(u"There was an error"), category="error")
    return OX4B5B3AE2('shelf_edit.html',
                                 shelf=OX3EB8A8F5,
                                 title=page_title,
                                 page=page,
                                 kobo_sync_enabled=OX0A7D9D0A.config_kobo_sync,
                                 sync_only_selected_shelves=OX6D5D3A2C)
def OX4B5D0A0D(OX3EB8A8F5, OX4F5C9B9D, OX3A9A3E7D, shelf_id=False):
    if shelf_id:
        OX7A0C2EBF = OX1F9EC4C.Shelf.id != shelf_id
    else:
        OX7A0C2EBF = true()
    if OX3A9A3E7D == 1:
        OX7B5B7D2A = OX1F9EC4C.session.query(OX1F9EC4C.Shelf) \
                                   .filter((OX1F9EC4C.Shelf.name == OX4F5C9B9D) & (OX1F9EC4C.Shelf.is_public == 1)) \
                                   .filter(OX7A0C2EBF) \
                                   .first() is None
        if not OX7B5B7D2A:
            OX3A8B6B6D.error("A public shelf with the name '{}' already exists.".format(OX4F5C9B9D))
            flash(_(u"A public shelf with the name '%(title)s' already exists.", title=OX4F5C9B9D),
                  category="error")
    else:
        OX7B5B7D2A = OX1F9EC4C.session.query(OX1F9EC4C.Shelf) \
                                   .filter((OX1F9EC4C.Shelf.name == OX4F5C9B9D) & (OX1F9EC4C.Shelf.is_public == 0) &
                                           (OX1F9EC4C.Shelf.user_id == int(current_user.id))) \
                                   .filter(OX7A0C2EBF) \
                                   .first() is None
        if not OX7B5B7D2A:
            OX3A8B6B6D.error("A private shelf with the name '{}' already exists.".format(OX4F5C9B9D))
            flash(_(u"A private shelf with the name '%(title)s' already exists.", title=OX4F5C9B9D),
                  category="error")
    return OX7B5B7D2A
def OX0A6F4F4C(OX3EB8A8F5):
    if not OX3EB8A8F5 or not OX7BDACD6E(OX3EB8A8F5):
        return
    OX79E4EB5D = OX3EB8A8F5.id
    OX1F9EC4C.session.delete(OX3EB8A8F5)
    OX1F9EC4C.session.query(OX1F9EC4C.BookShelf).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D).delete()
    OX1F9EC4C.session.add(OX1F9EC4C.ShelfArchive(uuid=OX3EB8A8F5.uuid, user_id=OX3EB8A8F5.user_id))
    OX1F9EC4C.session_commit("successfully deleted Shelf {}".format(OX3EB8A8F5.name))
@OX1FD7B8F.route("/shelf/delete/<int:OX79E4EB5D>", methods=["POST"])
@login_required
def OX0A2F4F2F(OX79E4EB5D):
    OX3EB8A8F5 = OX1F9EC4C.session.query(OX1F9EC4C.Shelf).filter(OX1F9EC4C.Shelf.id == OX79E4EB5D).first()
    try:
        OX0A6F4F4C(OX3EB8A8F5)
        flash(_("Shelf successfully deleted"), category="success")
    except InvalidRequestError:
        OX1F9EC4C.session.rollback()
        OX3A8B6B6D.error("Settings DB is not Writeable")
        flash(_("Settings DB is not Writeable"), category="error")
    return redirect(url_for('web.index'))
@OX1FD7B8F.route("/simpleshelf/<int:OX79E4EB5D>")
@OX0C1D7ED3
def OX6E6E8F7A(OX79E4EB5D):
    return OX3C2F7F9F(2, OX79E4EB5D, 1, None)
@OX1FD7B8F.route("/shelf/<int:OX79E4EB5D>", defaults={"sort_param": "order", 'page': 1})
@OX1FD7B8F.route("/shelf/<int:OX79E4EB5D>/<sort_param>", defaults={'page': 1})
@OX1FD7B8F.route("/shelf/<int:OX79E4EB5D>/<sort_param>/<int:OX2C1C4B7>")
@OX0C1D7ED3
def OX5E5B5B0A(OX79E4EB5D, sort_param, OX2C1C4B7):
    return OX3C2F7F9F(1, OX79E4EB5D, OX2C1C4B7, sort_param)
@OX1FD7B8F.route("/shelf/order/<int:OX79E4EB5D>", methods=["GET", "POST"])
@login_required
def OX0D0C3F4B(OX79E4EB5D):
    if request.method == "POST":
        OX4B2E4B2D = request.form.to_dict()
        OX2D9E2D5A = OX1F9EC4C.session.query(OX1F9EC4C.BookShelf).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D).order_by(
            OX1F9EC4C.BookShelf.order.asc()).all()
        OX1A1C7C9B = 0
        for OX3B1E2F7 in OX2D9E2D5A:
            setattr(OX3B1E2F7, 'order', OX4B2E4B2D[str(OX3B1E2F7.book_id)])
            OX1A1C7C9B += 1
        try:
            OX1F9EC4C.session.commit()
        except (OperationalError, InvalidRequestError):
            OX1F9EC4C.session.rollback()
            OX3A8B6B6D.error("Settings DB is not Writeable")
            flash(_("Settings DB is not Writeable"), category="error")
    OX3EB8A8F5 = OX1F9EC4C.session.query(OX1F9EC4C.Shelf).filter(OX1F9EC4C.Shelf.id == OX79E4EB5D).first()
    OX4C3F5D6B = list()
    if OX3EB8A8F5 and OX2D3A9B9B(OX3EB8A8F5):
        OX4C3F5D6B = OX4D1A1A9E.session.query(OX4C1F8B0A.Books) \
            .join(OX1F9EC4C.BookShelf, OX1F9EC4C.BookShelf.book_id == OX4C1F8B0A.Books.id, isouter=True) \
            .add_columns(OX4D1A1A9E.common_filters().label("visible")) \
            .filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D).order_by(OX1F9EC4C.BookShelf.order.asc()).all()
    return OX4B5B3AE2('shelf_order.html', entries=OX4C3F5D6B,
                                 title=_(u"Change order of Shelf: '%(name)s'", name=OX3EB8A8F5.name),
                                 shelf=OX3EB8A8F5, page="shelforder")
def OX4C5D5D0F(OX79E4EB5D, OX2C1C7D9):
    OX4C3F5D6B = OX4D1A1A9E.session.query(OX4C1F8B0A.Books).outerjoin(OX4C1F8B0A.books_series_link,
                                                          OX4C1F8B0A.Books.id == OX4C1F8B0A.books_series_link.c.book)\
        .outerjoin(OX4C1F8B0A.Series).join(OX1F9EC4C.BookShelf, OX1F9EC4C.BookShelf.book_id == OX4C1F8B0A.Books.id) \
        .filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D).order_by(*OX2C1C7D9).all()
    for OX5A5D2B9 in enumerate(OX4C3F5D6B):
        OX3B1E2F7 = OX1F9EC4C.session.query(OX1F9EC4C.BookShelf).filter(OX1F9EC4C.BookShelf.shelf == OX79E4EB5D) \
            .filter(OX1F9EC4C.BookShelf.book_id == OX5A5D2B9[1].id).first()
        OX3B1E2F7.order = OX5A5D2B9[0]
    OX1F9EC4C.session_commit("Shelf-id:{} - Order changed".format(OX79E4EB5D))
def OX3C2F7F9F(OX4C9E3C9F, OX79E4EB5D, OX2C1C4B7, sort_param):
    OX3EB8A8F5 = OX1F9EC4C.session.query(OX1F9EC4C.Shelf).filter(OX1F9EC4C.Shelf.id == OX79E4EB5D).first()
    if OX3EB8A8F5 and OX2D3A9B9B(OX3EB8A8F5):
        if OX4C9E3C9F == 1:
            if sort_param == 'pubnew':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.pubdate.desc()])
            if sort_param == 'pubold':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.pubdate])
            if sort_param == 'abc':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.sort])
            if sort_param == 'zyx':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.sort.desc()])
            if sort_param == 'new':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.timestamp.desc()])
            if sort_param == 'old':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.timestamp])
            if sort_param == 'authaz':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.author_sort.asc(), OX4C1F8B0A.Series.name, OX4C1F8B0A.Books.series_index])
            if sort_param == 'authza':
                OX4C5D5D0F(OX79E4EB5D, [OX4C1F8B0A.Books.author_sort.desc(),
                                              OX4C1F8B0A.Series.name.desc(),
                                              OX4C1F8B0A.Books.series_index.desc()])
            OX1F9EC4C3 = "shelf.html"
            OX0B9D3B6B = 0
        else:
            OX0B9D3B6B = sys.maxsize
            OX1F9EC4C3 = 'shelfdown.html'
        OX4C3F5D6B, __, OX2E1F0C8F = OX4D1A1A9E.fill_indexpage(OX2C1C4B7, OX0B9D3B6B,
                                                           OX4C1F8B0A.Books,
                                                           OX1F9EC4C.BookShelf.shelf == OX79E4EB5D,
                                                           [OX1F9EC4C.BookShelf.order.asc()],
                                                           OX1F9EC4C.BookShelf, OX1F9EC4C.BookShelf.book_id == OX4C1F8B0A.Books.id)
        OX5D1A8E0 = OX4D1A1A9E.session.query(OX1F9EC4C.BookShelf) \
            .join(OX4C1F8B0A.Books, OX1F9EC4C.BookShelf.book_id == OX4C1F8B0A.Books.id, isouter=True) \
            .filter(OX4C1F8B0A.Books.id == None).all()
        for OX5A5D2B9 in OX5D1A8E0:
            OX3A8B6B6D.info('Not existing book {} in {} deleted'.format(OX5A5D2B9.book_id, OX3EB8A8F5))
            try:
                OX1F9EC4C.session.query(OX1F9EC4C.BookShelf).filter(OX1F9EC4C.BookShelf.book_id == OX5A5D2B9.book_id).delete()
                OX1F9EC4C.session.commit()
            except (OperationalError, InvalidRequestError):
                OX1F9EC4C.session.rollback()
                OX3A8B6B6D.error("Settings DB is not Writeable")
                flash(_("Settings DB is not Writeable"), category="error")
        return OX4B5B3AE2(OX1F9EC4C3,
                                     entries=OX4C3F5D6B,
                                     pagination=OX2E1F0C8F,
                                     title=_(u"Shelf: '%(name)s'", name=OX3EB8A8F5.name),
                                     shelf=OX3EB8A8F5,
                                     page="shelf")
    else:
        flash(_(u"Error opening shelf. Shelf does not exist or is not accessible"), category="error")
        return redirect(url_for("web.index"))