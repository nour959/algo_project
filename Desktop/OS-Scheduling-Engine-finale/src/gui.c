#include <gtk/gtk.h>
#include <pthread.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/process.h"
#include "../includes/simulation.h"

typedef struct {
    Process *processes;
    int num_processes;
    SchedulerFunc scheduler;
    int quantum;
    GtkWidget *status_label;
    GtkWidget *drawing_area;
    GtkWidget *progress_bar;
    GtkWidget *stats_label;
} SimulationData;

static SimulationData g_sim_data = {0};

#define BG_COLOR        0.13, 0.15, 0.18
#define FG_COLOR        0.95, 0.95, 0.97

static gboolean update_gui_after_simulation(gpointer user_data) {
    (void)user_data;
    if (!g_sim_data.processes || g_sim_data.num_processes == 0) return FALSE;

    double total_wait = 0, total_turnaround = 0;
    for (int i = 0; i < g_sim_data.num_processes; i++) {
        total_wait += g_sim_data.processes[i].waiting_time;
        total_turnaround += g_sim_data.processes[i].turnaround_time;
    }

    char stats[256];
    snprintf(stats, sizeof(stats),
        "Temps moyen d'attente : %.2f  |  Temps moyen de rotation : %.2f",
        total_wait / g_sim_data.num_processes,
        total_turnaround / g_sim_data.num_processes);

    gtk_label_set_text(GTK_LABEL(g_sim_data.stats_label), stats);
    gtk_label_set_text(GTK_LABEL(g_sim_data.status_label), "Simulation terminée");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_sim_data.progress_bar), 1.0);
    gtk_widget_queue_draw(g_sim_data.drawing_area);
    return FALSE;
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data) {
    (void)data;
    if (!g_sim_data.processes || g_sim_data.num_processes == 0) return FALSE;

    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    double width = alloc.width;

    cairo_set_source_rgb(cr, BG_COLOR);
    cairo_paint(cr);

    int max_time = 0;
    for (int i = 0; i < g_sim_data.num_processes; i++)
        if (g_sim_data.processes[i].finish_time > max_time)
            max_time = g_sim_data.processes[i].finish_time;

    double margin_left = 160, margin_top = 120;
    double bar_height = 48, space_between = 22;
    double usable_width = width - margin_left - 60;
    double scale = (max_time > 0) ? usable_width / (double)max_time : 1.0;

    cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
    cairo_set_line_width(cr, 5.0);
    cairo_move_to(cr, margin_left, margin_top - 30);
    cairo_line_to(cr, width - 40, margin_top - 30);
    cairo_stroke(cr);

    cairo_set_font_size(cr, 14);
    for (int t = 0; t <= max_time; t += 2) {
        double x = margin_left + t * scale;
        cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
        cairo_move_to(cr, x, margin_top - 40);
        cairo_line_to(cr, x, margin_top - 15);
        cairo_stroke(cr);

        char buf[16];
        snprintf(buf, sizeof(buf), "%d", t);
        cairo_text_extents_t te;
        cairo_text_extents(cr, buf, &te);
        cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
        cairo_move_to(cr, x - te.width/2, margin_top - 50);
        cairo_show_text(cr, buf);
    }

    const double colors[10][3] = {
        {0.10, 0.70, 0.90}, {0.95, 0.35, 0.45}, {0.30, 0.85, 0.55},
        {0.80, 0.40, 0.95}, {1.00, 0.75, 0.20}, {0.15, 0.80, 0.90},
        {0.90, 0.25, 0.35}, {0.55, 0.40, 0.95}, {1.00, 0.60, 0.40}, {0.25, 0.70, 0.95}
    };

    for (int i = 0; i < g_sim_data.num_processes; i++) {
        double y = margin_top + i * (bar_height + space_between);
        Process *p = &g_sim_data.processes[i];

        cairo_set_source_rgb(cr, FG_COLOR);
        cairo_set_font_size(cr, 18);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_move_to(cr, 20, y + bar_height/2 + 8);
        cairo_show_text(cr, p->name);

        double r = colors[i % 10][0], g = colors[i % 10][1], b = colors[i % 10][2];

        for (int s = 0; s < p->num_slices; s++) {
            double x_start = margin_left + p->slices[s].start * scale;
            double w = p->slices[s].duration * scale;
            if (w < 3) w = 3;

            cairo_set_source_rgba(cr, 0, 0, 0, 0.25);
            cairo_rectangle(cr, x_start + 3, y + 3, w, bar_height);
            cairo_fill(cr);

            double radius = bar_height / 2.2;
            cairo_set_source_rgb(cr, r, g, b);
            cairo_move_to(cr, x_start, y + radius);
            cairo_arc(cr, x_start + radius, y + radius, radius, G_PI, 3*G_PI/2);
            cairo_arc(cr, x_start + w - radius, y + radius, radius, 3*G_PI/2, 0);
            cairo_arc(cr, x_start + w - radius, y + bar_height - radius, radius, 0, G_PI/2);
            cairo_arc(cr, x_start + radius, y + bar_height - radius, radius, G_PI/2, G_PI);
            cairo_close_path(cr);
            cairo_fill_preserve(cr);
            cairo_set_source_rgb(cr, r*0.7, g*0.7, b*0.7);
            cairo_set_line_width(cr, 3.5);
            cairo_stroke(cr);

            if (w > 50) {
                cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
                cairo_set_font_size(cr, 15);
                cairo_text_extents_t te;
                cairo_text_extents(cr, p->name, &te);
                cairo_move_to(cr, x_start + w/2 - te.width/2, y + bar_height/2 + 7);
                cairo_show_text(cr, p->name);
            }
        }
    }
    return FALSE;
}

static void* run_scheduler_thread(void *arg) {
    (void)arg;
    g_sim_data.scheduler(g_sim_data.processes, g_sim_data.num_processes, g_sim_data.quantum);
    display_results(g_sim_data.processes, g_sim_data.num_processes);
    g_idle_add(update_gui_after_simulation, NULL);
    return NULL;
}

static void on_add_process_clicked(GtkWidget *button, gpointer user_data) {
    (void)button;
    GtkWidget *grid = (GtkWidget*)user_data;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Ajouter un processus", NULL, GTK_DIALOG_MODAL,
                                                    "Annuler", GTK_RESPONSE_CANCEL,
                                                    "Ajouter", GTK_RESPONSE_OK, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g), 15);
    gtk_box_pack_start(GTK_BOX(content), g, TRUE, TRUE, 10);

    GtkWidget *e_name = gtk_entry_new();
    GtkWidget *e_arr  = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_arr),  "0");
    GtkWidget *e_bur  = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_bur),  "5");
    GtkWidget *e_pri  = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_pri),  "1");

    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Nom :"),         0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g), e_name,                         1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Arrivée :"),    0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(g), e_arr,                         1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Durée :"),       0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(g), e_bur,                         1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Priorité :"),    0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(g), e_pri,                         1, 3, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(e_name));
        int arr = atoi(gtk_entry_get_text(GTK_ENTRY(e_arr)));
        int bur = atoi(gtk_entry_get_text(GTK_ENTRY(e_bur)));
        int pri = atoi(gtk_entry_get_text(GTK_ENTRY(e_pri)));

        if (strlen(name) > 0 && bur > 0 && g_sim_data.num_processes < MAX_PROCESSES) {
            Process *p = &g_sim_data.processes[g_sim_data.num_processes++];
            strncpy(p->name, name, MAX_NAME_LENGTH-1); p->name[MAX_NAME_LENGTH-1] = '\0';
            p->arrival_time = arr; p->burst_time = bur; p->priority = pri;
            p->remaining_time = bur; p->start_time = -1; p->finish_time = 0;
            p->is_started = 0; p->num_slices = 0;

            int row = g_sim_data.num_processes;
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(p->name), 0, row, 1, 1);
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", arr);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(buf), 1, row, 1, 1);
            snprintf(buf, sizeof(buf), "%d", bur);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(buf), 2, row, 1, 1);
            snprintf(buf, sizeof(buf), "%d", pri);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(buf), 3, row, 1, 1);

            gtk_widget_show_all(grid);
        }
    }
    gtk_widget_destroy(dialog);
}

static void on_manual_entry_clicked(GtkWidget *button, gpointer user_data) {
    (void)button; (void)user_data;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Saisie manuelle des processus",
        NULL, GTK_DIALOG_MODAL, "Fermer", GTK_RESPONSE_CANCEL,
        "Lancer la simulation", GTK_RESPONSE_OK, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled, 700, 450);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 40);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);

    const char *headers[] = {"<b>Nom</b>", "<b>Arrivée</b>", "<b>Durée</b>", "<b>Priorité</b>"};
    for (int i = 0; i < 4; i++) {
        GtkWidget *h = gtk_label_new(""); gtk_label_set_markup(GTK_LABEL(h), headers[i]);
        gtk_grid_attach(GTK_GRID(grid), h, i, 0, 1, 1);
    }

    gtk_container_add(GTK_CONTAINER(scrolled), grid);
    gtk_box_pack_start(GTK_BOX(content), scrolled, TRUE, TRUE, 10);

    GtkWidget *add_btn = gtk_button_new_with_label("Ajouter un processus");
    gtk_box_pack_start(GTK_BOX(content), add_btn, FALSE, FALSE, 10);

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add_process_clicked), grid);

    gtk_widget_show_all(dialog);
    g_sim_data.num_processes = 0;

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK && g_sim_data.num_processes > 0) {
        gtk_label_set_text(GTK_LABEL(g_sim_data.status_label), "Simulation en cours...");
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_sim_data.progress_bar), 0.0);
        pthread_t thread;
        pthread_create(&thread, NULL, run_scheduler_thread, NULL);
        pthread_detach(thread);
    }
    gtk_widget_destroy(dialog);
}

void run_gui_simulation(Process processes[], int num_processes, SchedulerFunc scheduler, int quantum) {
    if (processes != NULL && num_processes > 0) {
        g_sim_data.processes = processes;
        g_sim_data.num_processes = num_processes;
        g_sim_data.scheduler = scheduler;
        g_sim_data.quantum = quantum;
    }

    gtk_init(NULL, NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Ordonnanceur de processus - Systèmes d'exploitation");
    gtk_window_set_default_size(GTK_WINDOW(window), 1500, 950);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    gtk_container_set_border_width(GTK_CONTAINER(window), 30);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='x-large' weight='bold'>Ordonnanceur de processus</span>");
    gtk_box_pack_start(GTK_BOX(main_box), title, FALSE, FALSE, 0);

    g_sim_data.status_label = gtk_label_new("Prêt");
    gtk_box_pack_start(GTK_BOX(main_box), g_sim_data.status_label, FALSE, FALSE, 0);

    g_sim_data.progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(main_box), g_sim_data.progress_bar, FALSE, FALSE, 10);

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 100);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box, FALSE, FALSE, 20);

    GtkWidget *file_btn = gtk_button_new_with_label("Charger depuis un fichier");
    GtkWidget *manual_btn = gtk_button_new_with_label("Saisie manuelle");

    gtk_widget_set_size_request(file_btn, 320, 65);
    gtk_widget_set_size_request(manual_btn, 320, 65);

    gtk_box_pack_start(GTK_BOX(btn_box), file_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), manual_btn, FALSE, FALSE, 0);

    g_signal_connect(file_btn, "clicked", G_CALLBACK(run_scheduler_thread), NULL);
    g_signal_connect(manual_btn, "clicked", G_CALLBACK(on_manual_entry_clicked), NULL);

    g_sim_data.drawing_area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(main_box), g_sim_data.drawing_area, TRUE, TRUE, 0);
    g_signal_connect(g_sim_data.drawing_area, "draw", G_CALLBACK(on_draw_event), NULL);

    g_sim_data.stats_label = gtk_label_new("En attente de simulation...");
    gtk_box_pack_start(GTK_BOX(main_box), g_sim_data.stats_label, FALSE, FALSE, 10);

    gtk_widget_show_all(window);
    gtk_main();
}