import mido
import serial
import serial.tools.list_ports
import customtkinter as ctk
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import threading

class TeensyControllerApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        # --- CONFIGURATION FENÊTRE ---
        self.title("Teensy: EQ & Visualizer")
        self.geometry("1200x850")
        ctk.set_appearance_mode("Dark")
        
        # --- VARIABLES D'ÉTAT ---
        self.midi_out = None
        self.serial_in = None
        self.running = True
        
        # Données EQ (5 Bandes)
        self.freqs_val = [80, 330, 410, 660, 910]
        self.freqs_labels = ["80Hz", "330Hz", "410Hz", "660Hz", "910Hz"]
        self.gains = [0.0] * 5
        self.sliders = []
        self.labels_db = []
        
        # Données Visualizer (5 Boutons)
        # Couleurs: Vert, Rouge, Jaune, Bleu, Orange
        self.btn_colors = ["#00FF00", "#FF0000", "#FFFF00", "#0088FF", "#FFA500"]
        self.bubbles = []

        # --- INIT CONNEXIONS ---
        self.connect_midi()
        self.connect_serial()

        # --- CONSTRUCTION UI ---
        self.setup_ui()
        self.draw_curve() # Premier tracé

        # --- THREAD SÉRIE ---
        # Écoute la Teensy en arrière-plan pour récupérer les notes
        self.thread = threading.Thread(target=self.read_serial_loop, daemon=True)
        self.thread.start()

        # Gestion fermeture propre
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

    def setup_ui(self):
        # 1. HEADER
        ctk.CTkLabel(self, text="Teensy Control Center", font=("Roboto", 24, "bold")).pack(pady=10)

        # 2. ZONE HAUTE (Visualizer + Graphique)
        top_frame = ctk.CTkFrame(self, fg_color="transparent")
        top_frame.pack(fill="x", padx=20, pady=5)

        # A. GAUCHE : NOTES JOUÉES (Guitar Hero Style)
        notes_frame = ctk.CTkFrame(top_frame, width=300, height=350)
        notes_frame.pack(side="left", fill="y", padx=(0, 15))
        notes_frame.pack_propagate(False)

        ctk.CTkLabel(notes_frame, text="NOTES JOUÉES", font=("Roboto", 14, "bold")).pack(pady=15)
        
        # Canvas pour les 5 bulles
        self.note_canvas = ctk.CTkCanvas(notes_frame, width=260, height=60, bg="#2b2b2b", highlightthickness=0)
        self.note_canvas.pack(pady=20)
        
        # Création des 5 cercles (éteints par défaut)
        for i in range(5):
            x = 30 + (i * 50)
            bubble = self.note_canvas.create_oval(
                x - 15, 30 - 15, x + 15, 30 + 15,
                fill="#333333", outline=self.btn_colors[i], width=2
            )
            self.bubbles.append(bubble)

        # Nom de l'accord
        self.chord_label = ctk.CTkLabel(notes_frame, text="--", font=("Roboto", 32, "bold"), text_color="white")
        self.chord_label.pack(pady=30)
        
        # B. DROITE : GRAPHIQUE FRÉQUENCE
        graph_frame = ctk.CTkFrame(top_frame)
        graph_frame.pack(side="right", fill="both", expand=True)

        self.fig, self.ax = plt.subplots(figsize=(6, 3.5), facecolor='#2b2b2b')
        self.ax.set_facecolor('#1a1a1a')
        self.ax.set_xlim(20, 20000)
        self.ax.set_ylim(-20, 20)
        self.ax.set_xscale('log')
        self.ax.grid(True, which='both', linestyle='--', alpha=0.2)
        self.ax.tick_params(colors='white', labelsize=8)
        self.line, = self.ax.plot([], [], color='#3B8ED0', lw=2)
        
        self.canvas_plot = FigureCanvasTkAgg(self.fig, master=graph_frame)
        self.canvas_plot.get_tk_widget().pack(fill="both", expand=True, padx=5, pady=5)

        # 3. ZONE BASSE (Contrôles EQ)
        eq_frame = ctk.CTkFrame(self)
        eq_frame.pack(fill="both", expand=True, padx=20, pady=15)
        
        # Entête EQ
        header_eq = ctk.CTkFrame(eq_frame, fg_color="transparent")
        header_eq.pack(fill="x", pady=5)
        ctk.CTkLabel(header_eq, text="ÉGALISEUR 5 BANDES", font=("Roboto", 16, "bold")).pack(side="left", padx=20)
        ctk.CTkButton(header_eq, text="Reset", width=60, command=self.reset_filters, fg_color="#444").pack(side="left")

        # Sliders
        sliders_container = ctk.CTkFrame(eq_frame, fg_color="transparent")
        sliders_container.pack(fill="both", expand=True, pady=10)

        for i in range(5):
            col = ctk.CTkFrame(sliders_container, fg_color="transparent")
            col.pack(side="left", fill="both", expand=True)
            
            ctk.CTkLabel(col, text=self.freqs_labels[i], font=("Roboto", 11)).pack()
            s = ctk.CTkSlider(col, orientation="vertical", from_=0, to=127, 
                              command=lambda v, idx=i: self.update_filter(idx, v))
            s.set(64)
            s.pack(expand=True, fill="y", pady=10)
            
            lbl = ctk.CTkLabel(col, text="0.0 dB", font=("Roboto", 10))
            lbl.pack()
            self.labels_db.append(lbl)
            self.sliders.append(s)

        # Master Vol (Sidebar à droite des sliders)
        master_col = ctk.CTkFrame(sliders_container, width=100)
        master_col.pack(side="right", fill="y", padx=10)
        ctk.CTkLabel(master_col, text="MASTER", font=("Roboto", 12, "bold")).pack(pady=10)
        self.master_s = ctk.CTkSlider(master_col, orientation="vertical", from_=0, to=127, command=self.update_master)
        self.master_s.set(64)
        self.master_s.pack(expand=True, pady=10)

    # --- LOGIQUE MÉTIER ---

    def update_filter(self, idx, val):
        # 1. Calcul dB
        db = round(((float(val) - 64) / 64) * 20.0, 1)
        self.gains[idx] = db
        self.labels_db[idx].configure(text=f"{db} dB")
        
        # 2. Envoi MIDI vers Teensy (CC 21 à 26)
        if self.midi_out:
            self.midi_out.send(mido.Message('control_change', control=21+idx, value=int(val)))
            
        # 3. Mise à jour graphique
        self.draw_curve()

    def update_master(self, val):
        if self.midi_out:
            self.midi_out.send(mido.Message('control_change', control=7, value=int(val)))

    def reset_filters(self):
        for i in range(6):
            self.sliders[i].set(64)
            self.update_filter(i, 64)

    def draw_curve(self):
        # Simulation réponse EQ
        x = np.logspace(np.log10(20), np.log10(20000), 200)
        y = np.zeros_like(x)
        for i, f_c in enumerate(self.freqs_val):
            # Formule approximative Bell Filter
            width = f_c / 1.2
            y += self.gains[i] * np.exp(-0.5 * ((x - f_c) / (width/2))**2)
        
        self.line.set_data(x, y)
        self.canvas_plot.draw_idle()

    # --- GESTION DES NOTES (SÉRIE) ---
    def read_serial_loop(self):
        """Lit le port série en boucle pour récupérer les notes envoyées par la Teensy"""
        while self.running:
            if self.serial_in and self.serial_in.is_open:
                try:
                    if self.serial_in.in_waiting:
                        # Lecture de la ligne envoyée par Teensy (ex: "N:5:Do Majeur")
                        line = self.serial_in.readline().decode('utf-8', errors='ignore').strip()
                        if line.startswith("N:"):
                            parts = line.split(":")
                            if len(parts) >= 3:
                                mask = int(parts[1])
                                chord = parts[2]
                                # Mise à jour UI (Thread-safe via after ou direct customtkinter)
                                self.update_bubbles(mask, chord)
                except Exception as e:
                    print(f"Erreur Série: {e}")
            else:
                # Si pas connecté, on attend un peu pour éviter de boucler à l'infini
                threading.Event().wait(1.0)

    def update_bubbles(self, mask, chord_name):
        self.chord_label.configure(text=chord_name)
        # Bitmask : Vert=1, Rouge=2, Jaune=4, Bleu=8, Orange=16
        for i in range(5):
            is_on = (mask >> i) & 1
            color = self.btn_colors[i] if is_on else "#333333"
            self.note_canvas.itemconfig(self.bubbles[i], fill=color)

    # --- CONNEXIONS MATÉRIELLES ---
    def connect_midi(self):
        try:
            name = next((p for p in mido.get_output_names() if "Teensy" in p), None)
            if name:
                self.midi_out = mido.open_output(name)
                print(f"MIDI Connecté: {name}")
            else:
                print("MIDI: Teensy non trouvée")
        except: pass

    def connect_serial(self):
        try:
            # Recherche auto port COM
            ports = serial.tools.list_ports.comports()
            t_port = next((p.device for p in ports if "Teensy" in p.description or "Serial" in p.description), None)
            if not t_port and ports: t_port = ports[0].device # Fallback

            if t_port:
                self.serial_in = serial.Serial(t_port, 9600, timeout=1)
                print(f"Série Connecté: {t_port}")
            else:
                print("Série: Teensy non trouvée")
        except Exception as e:
            print(f"Erreur Connexion Série: {e}")

    def on_closing(self):
        self.running = False
        if self.midi_out: self.midi_out.close()
        if self.serial_in: self.serial_in.close()
        self.destroy()

if __name__ == "__main__":
    app = TeensyControllerApp()
    app.mainloop()