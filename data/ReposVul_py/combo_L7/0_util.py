#include <thread>
#include <future>
#include <chrono>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <string>
#include <iostream>
#include <cmath>
#include <fstream>
#include <mutex>
#include <condition_variable>

extern "C" {
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

using namespace std;

// Inline assembly function to get current thread id (simulating Python's threading module)
inline uint64_t get_thread_id() {
    uint64_t tid;
    __asm__ __volatile__("mov %%fs:0x10, %0" : "=r" (tid));
    return tid;
}

enum TransferDirection {
    TO_REMOTE_MACHINE,
    FROM_REMOTE_MACHINE
};

enum FileType {
    REGULAR = G_FILE_TYPE_REGULAR,
    DIRECTORY = G_FILE_TYPE_DIRECTORY,
    SYMBOLIC_LINK = G_FILE_TYPE_SYMBOLIC_LINK
};

enum RemoteStatus {
    ONLINE,
    OFFLINE,
    INIT_CONNECTING,
    UNREACHABLE,
    AWAITING_DUPLEX
};

enum OpStatus {
    INIT,
    CALCULATING,
    WAITING_PERMISSION,
    CANCELLED_PERMISSION_BY_SENDER,
    CANCELLED_PERMISSION_BY_RECEIVER,
    TRANSFERRING,
    PAUSED,
    STOPPED_BY_SENDER,
    STOPPED_BY_RECEIVER,
    FAILED,
    FAILED_UNRECOVERABLE,
    FILE_NOT_FOUND,
    FINISHED
};

enum OpCommand {
    START_TRANSFER,
    UPDATE_PROGRESS,
    CANCEL_PERMISSION_BY_SENDER,
    CANCEL_PERMISSION_BY_RECEIVER,
    PAUSE_TRANSFER,
    RETRY_TRANSFER,
    STOP_TRANSFER_BY_SENDER,
    STOP_TRANSFER_BY_RECEIVER,
    REMOVE_TRANSFER
};

class InterfaceInfo {
    string ip4_address;
    string ip6_address;
    public:
    InterfaceInfo(string ip4, string ip6) : ip4_address(ip4), ip6_address(ip6) {}
    bool operator==(const InterfaceInfo &other) const {
        return ip4_address == other.ip4_address;
    }
};

class RemoteInterfaceInfo {
    string ip4_address;
    string ip6_address;
    public:
    RemoteInterfaceInfo(vector<string> blist) {
        for (const auto& item : blist) {
            try {
                ip4_address = inet_ntop(AF_INET, item.c_str(), nullptr, 0);
            } catch (...) {
                ip6_address = inet_ntop(AF_INET6, item.c_str(), nullptr, 0);
            }
        }
    }

    bool operator==(const RemoteInterfaceInfo &other) const {
        return ip4_address == other.ip4_address;
    }
};

GFile *last_location = g_file_new_for_path(g_get_home_dir());

class FolderFileChooserDialog {
    GtkWidget *dialog;
    GtkWidget *chooser;
    public:
    FolderFileChooserDialog(const char *window_title, GtkWindow *transient_parent, GFile *starting_location) {
        dialog = gtk_dialog_new_with_buttons(window_title, transient_parent, GTK_DIALOG_MODAL, _("Cancel"), GTK_RESPONSE_CANCEL, _("Add"), GTK_RESPONSE_OK, NULL);
        chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
        gtk_file_chooser_set_current_folder_file(GTK_FILE_CHOOSER(chooser), starting_location, nullptr);
        gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), chooser);
    }
};

void create_file_and_folder_picker(GtkWindow *dialog_parent = nullptr) {
    FolderFileChooserDialog chooser(_("Select file(s) to send"), dialog_parent, last_location);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    create_file_and_folder_picker();
    return 0;
}