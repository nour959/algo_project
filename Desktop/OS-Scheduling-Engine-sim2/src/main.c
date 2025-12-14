
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <ctype.h>      

#include "../includes/process.h"
#include "../includes/parser.h"
#include "../includes/simulation.h"

extern void run_gui_simulation(Process processes[], int num_processes, SchedulerFunc scheduler, int quantum);

Process g_processes[MAX_PROCESSES];
int g_num_processes = 0;
SchedulerFunc g_scheduler = NULL;
int g_quantum = 4;

GtkWidget *quantum_entry = NULL;

typedef struct {
    char display_name[64];
    char so_path[256];
    char func_name[128];
} Policy;

Policy *policies = NULL;
int num_policies = 0;

void load_policies_from_directory() {
    DIR *dir = opendir("policies");
    if (!dir) {
        fprintf(stderr, "Erreur : dossier 'policies' introuvable\n");
        return;
    }

    policies = calloc(50, sizeof(Policy));
    num_policies = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && num_policies < 50) {
        if (!strstr(entry->d_name, ".so") || entry->d_name[0] == '.') continue;

        snprintf(policies[num_policies].so_path, sizeof(policies[num_policies].so_path),
                "policies/%s", entry->d_name);

        char base[128];
        strncpy(base, entry->d_name, sizeof(base)-1);
        base[sizeof(base)-1] = '\0';
        char *dot = strrchr(base, '.');
        if (dot) *dot = '\0';

        snprintf(policies[num_policies].func_name, sizeof(policies[num_policies].func_name),
                "schedule_%s", base);

        if (strcmp(base, "mlq") == 0) {

            strcpy(policies[num_policies].display_name, "Multilevel sans Aging");
        
        }
        else if (strcmp(base, "mlq_aging") == 0) {
        
            strcpy(policies[num_policies].display_name, "Multilevel avec Aging");
        
        }
        else {
            strcpy(policies[num_policies].display_name, base);
            if (strlen(policies[num_policies].display_name) > 0) {
                policies[num_policies].display_name[0] = toupper(policies[num_policies].display_name[0]);
            }
            for (size_t i = 0; i < strlen(policies[num_policies].display_name); i++) {
                if (policies[num_policies].display_name[i] == '_') {
                    policies[num_policies].display_name[i] = ' ';
                }
            }
        }
        

        num_policies++;
    }
    closedir(dir);
}

static void on_file_set(GtkFileChooserButton *chooser, GtkLabel *label) {
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
    if (!filename) return;

    if (parse_processes(filename, g_processes, &g_num_processes) == 0 && g_num_processes > 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Fichier chargé : %d processus", g_num_processes);
        gtk_label_set_text(label, msg);
    } else {
        gtk_label_set_text(label, "Erreur de chargement");
        g_num_processes = 0;
    }
    g_free(filename);
}

static void on_policy_changed(GtkComboBox *combo, gpointer user_data) {
    (void)user_data;
    int active = gtk_combo_box_get_active(combo);
    if (active < 0) return;

    const char *name = policies[active].display_name;

    char lower_name[64];
    strcpy(lower_name, name);
    for (int i = 0; lower_name[i]; i++) {
        lower_name[i] = tolower((unsigned char)lower_name[i]);
    }

    gboolean sensitive = (strstr(lower_name, "round") != NULL || 
                        strstr(lower_name, "robin") != NULL || 
                        strstr(lower_name, "multilevel") != NULL);

    gtk_widget_set_sensitive(quantum_entry, sensitive);
}

static void on_launch_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkComboBox *combo = GTK_COMBO_BOX(user_data);

    if (g_num_processes == 0) {
        GtkWidget *dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_OK, "Veuillez charger un fichier de processus !");
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return;
    }

    int idx = gtk_combo_box_get_active(combo);
    if (idx < 0) return;

    void *handle = dlopen(policies[idx].so_path, RTLD_LAZY);
    if (!handle) {
        GtkWidget *dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_OK, "Erreur dlopen : %s", dlerror());
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return;
    }

    dlerror();
    g_scheduler = (SchedulerFunc)dlsym(handle, policies[idx].func_name);
    const char *err = dlerror();
    if (err) {
        GtkWidget *dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_OK, "Erreur dlsym : %s", err);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        dlclose(handle);
        return;
    }

    const char *qtext = gtk_entry_get_text(GTK_ENTRY(quantum_entry));
    g_quantum = atoi(qtext);
    if (g_quantum <= 0) g_quantum = 4;

    run_gui_simulation(g_processes, g_num_processes, g_scheduler, g_quantum);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    load_policies_from_directory();
    if (num_policies == 0) {
        fprintf(stderr, "Aucune politique trouvée\n");
        return 1;
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Simulateur d'Ordonnancement de Processus");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_container_set_border_width(GTK_CONTAINER(window), 30);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title),
        "<span size='xx-large' weight='bold'>Simulateur d'Ordonnancement de Processus</span>");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *file_chooser = gtk_file_chooser_button_new("Choisir fichier de configuration", GTK_FILE_CHOOSER_ACTION_OPEN);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(file_chooser), filter);

    GtkWidget *file_label = gtk_label_new("Aucun fichier chargé");
    g_signal_connect(file_chooser, "file-set", G_CALLBACK(on_file_set), file_label);

    gtk_box_pack_start(GTK_BOX(box), file_chooser, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), file_label, FALSE, FALSE, 0);

    GtkWidget *policy_combo = gtk_combo_box_text_new();
    for (int i = 0; i < num_policies; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(policy_combo), policies[i].display_name);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(policy_combo), 0);
    g_signal_connect(policy_combo, "changed", G_CALLBACK(on_policy_changed), NULL);
    gtk_box_pack_start(GTK_BOX(box), policy_combo, FALSE, FALSE, 0);

    GtkWidget *qbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box), qbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(qbox), gtk_label_new("Quantum (Round-Robin & Multilevel sans Aging) :"), FALSE, FALSE, 0);
    quantum_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(quantum_entry), "4");
    gtk_widget_set_sensitive(quantum_entry, FALSE);
    gtk_box_pack_start(GTK_BOX(qbox), quantum_entry, FALSE, FALSE, 0);

    GtkWidget *launch = gtk_button_new_with_label("Lancer la simulation");
    gtk_widget_set_size_request(launch, 300, 60);
    g_signal_connect(launch, "clicked", G_CALLBACK(on_launch_clicked), policy_combo);
    gtk_box_pack_start(GTK_BOX(box), launch, FALSE, FALSE, 20);

    gtk_widget_show_all(window);
    gtk_main();

    free(policies);
    return 0;
}