import sys
import os
import signal
import mido
import serial
import serial.tools.list_ports
import sounddevice as sd  # <-- NOUVEAU : Pour l'audio
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
        self.audio_stream = None
        self.running = True
        
        # Données Audio pour l'oscilloscope
        self.chunk_size = 1024
        self.audio_data = np.zeros(self.chunk_size)
        
        # Données EQ (5 Bandes)
        self.freqs_val = [80, 330, 410, 660, 910]
        self.freqs_labels = ["80Hz", "330Hz", "410Hz", "660Hz", "910Hz"]
        self.gains = [0.0] * 5
        self.sliders = []
        self.labels_db = []
        
        # Données Visualizer (5 Boutons)
        self.btn_colors = ["#00FF00", "#FF0000", "#FFFF00", "#0088FF", "#FFA500"]
        self.bubbles = []

        # --- INIT CONNEXIONS ---
        self.connect_midi()
        self.connect_serial()
        self.connect_audio() # <-- NOUVEAU

        # --- CONSTRUCTION UI ---
        self.setup_ui()
        self.draw_curve()

        # --- THREADS & BOUCLES ---
        self.thread = threading.Thread(target=self.read_serial_loop, daemon=True)
        self.thread.start()
        
        # Démarre la boucle de rafraîchissement de l'oscilloscope
        self.after(1000, self.update_oscilloscope)

        # --- GESTION FERMETURE ---
        self.protocol("WM_DELETE_WINDOW", self.on_closing)
        signal.signal(signal.SIGINT, self.handle_sigint)
        self.poll_signals()

    def handle_sigint(self, sig, frame):
        print("\nFermeture demandée via le terminal...")
        self.on_closing()

    def poll_signals(self):
        self.after(200, self.poll_signals)

    def setup_ui(self):
        # 1. HEADER
        ctk.CTkLabel(self, text="Teensy Control Center", font=("Roboto", 24, "bold")).pack(pady=10)

        # 2. ZONE HAUTE (Visualizer + Graphiques)
        top_frame = ctk.CTkFrame(self, fg_color="transparent")
        top_frame.pack(fill="x", padx=20, pady=5)

        # A. GAUCHE : NOTES JOUÉES
        notes_frame = ctk.CTkFrame(top_frame, width=300, height=450)
        notes_frame.pack(side="left", fill="y", padx=(0, 15))
        notes_frame.pack_propagate(False)

        ctk.CTkLabel(notes_frame, text="NOTES JOUÉES", font=("Roboto", 14, "bold")).pack(pady=15)
        
        self.note_canvas = ctk.CTkCanvas(notes_frame, width=260, height=60, bg="#2b2b2b", highlightthickness=0)
        self.note_canvas.pack(pady=20)
        
        for i in range(5):
            x = 30 + (i * 50)
            bubble = self.note_canvas.create_oval(
                x - 15, 30 - 15, x + 15, 30 + 15,
                fill="#333333", outline=self.btn_colors[i], width=2
            )
            self.bubbles.append(bubble)

        self.chord_label = ctk.CTkLabel(notes_frame, text="--", font=("Roboto", 32, "bold"), text_color="white")
        self.chord_label.pack(pady=30)
        
        # B. DROITE : GRAPHIQUES (EQ + Oscilloscope)
        graph_frame = ctk.CTkFrame(top_frame)
        graph_frame.pack(side="right", fill="both", expand=True)

        # Création de 2 sous-graphiques superposés
        self.fig, (self.ax_eq, self.ax_osc) = plt.subplots(
            2, 1, 
            figsize=(6, 4.5), 
            facecolor='#2b2b2b',
            gridspec_kw={'height_ratios': [2, 1]} # L'EQ prend plus de place que l'oscilloscope
        )
        self.fig.subplots_adjust(hspace=0.4)

        # --- Graphique 1 : EQ ---
        self.ax_eq.set_facecolor('#1a1a1a')
        self.ax_eq.set_xlim(20, 20000)
        self.ax_eq.set_ylim(-20, 20)
        self.ax_eq.set_xscale('log')
        self.ax_eq.grid(True, which='both', linestyle='--', alpha=0.2)
        self.ax_eq.tick_params(colors='white', labelsize=8)
        self.ax_eq.set_title("Réponse en Fréquence (EQ)", color="gray", fontsize=10)
        self.line_eq, = self.ax_eq.plot([], [], color='#3B8ED0', lw=2)
        
        # --- Graphique 2 : Oscilloscope ---
        self.ax_osc.set_facecolor('#1a1a1a')
        self.ax_osc.set_xlim(0, self.chunk_size)
        self.ax_osc.set_ylim(-1, 1) # Plage audio standard (-1.0 à 1.0)
        self.ax_osc.grid(True, linestyle='--', alpha=0.2)
        self.ax_osc.tick_params(colors='white', labelsize=8)
        self.ax_osc.set_title("Signal Temporel (Audio USB)", color="gray", fontsize=10)
        # On masque les numéros de l'axe X pour que ce soit plus propre
        self.ax_osc.set_xticks([]) 
        self.line_osc, = self.ax_osc.plot(np.zeros(self.chunk_size), color='#00FF00', lw=1.5)

        self.canvas_plot = FigureCanvasTkAgg(self.fig, master=graph_frame)
        self.canvas_plot.get_tk_widget().pack(fill="both", expand=True, padx=5, pady=5)

        # 3. ZONE BASSE (Contrôles EQ)
        eq_frame = ctk.CTkFrame(self)
        eq_frame.pack(fill="both", expand=True, padx=20, pady=15)
        
        header_eq = ctk.CTkFrame(eq_frame, fg_color="transparent")
        header_eq.pack(fill="x", pady=5)
        ctk.CTkLabel(header_eq, text="ÉGALISEUR 5 BANDES", font=("Roboto", 16, "bold")).pack(side="left", padx=20)
        ctk.CTkButton(header_eq, text="Reset", width=60, command=self.reset_filters, fg_color="#444").pack(side="left")

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

        master_col = ctk.CTkFrame(sliders_container, width=100)
        master_col.pack(side="right", fill="y", padx=10)
        ctk.CTkLabel(master_col, text="MASTER", font=("Roboto", 12, "bold")).pack(pady=10)
        self.master_s = ctk.CTkSlider(master_col, orientation="vertical", from_=0, to=127, command=self.update_master)
        self.master_s.set(64)
        self.master_s.pack(expand=True, pady=10)

    # --- LOGIQUE MÉTIER EQ ---
    def update_filter(self, idx, val):
        db = round(((float(val) - 64) / 64) * 20.0, 1)
        self.gains[idx] = db
        self.labels_db[idx].configure(text=f"{db} dB")
        if self.midi_out:
            self.midi_out.send(mido.Message('control_change', control=21+idx, value=int(val)))
        self.draw_curve()

    def update_master(self, val):
        if self.midi_out:
            self.midi_out.send(mido.Message('control_change', control=7, value=int(val)))

    def reset_filters(self):
        for i in range(5):
            self.sliders[i].set(64)
            self.update_filter(i, 64)

    def draw_curve(self):
        x = np.logspace(np.log10(20), np.log10(20000), 200)
        y = np.zeros_like(x)
        for i, f_c in enumerate(self.freqs_val):
            width = f_c / 1.2
            y += self.gains[i] * np.exp(-0.5 * ((x - f_c) / (width/2))**2)
        self.line_eq.set_data(x, y)
        self.canvas_plot.draw_idle()

    # --- NOUVEAU : OSCILLOSCOPE (AUDIO) ---
    def audio_callback(self, indata, frames, time, status):
        """Fonction appelée automatiquement par sounddevice à chaque fois qu'un bout de son arrive."""
        if status:
            pass # Ignorer les petites erreurs de buffer (underflow/overflow)
        # On copie les données entrantes (canal 1) dans notre tableau
        self.audio_data = indata[:, 0]

    def update_oscilloscope(self):
        """Mise à jour graphique fluide, appelée toutes les 30ms."""
        if self.running:
            # Met à jour la ligne verte de l'oscilloscope avec les dernières données audio
            self.line_osc.set_ydata(self.audio_data)
            self.canvas_plot.draw_idle()
            # Rappelle cette fonction dans 30ms (~33 images par secondes)
            self.after(80, self.update_oscilloscope)

    # --- GESTION DES NOTES (SÉRIE) ---
    def read_serial_loop(self):
        while self.running:
            if self.serial_in and self.serial_in.is_open:
                try:
                    if self.serial_in.in_waiting:
                        line = self.serial_in.readline().decode('utf-8', errors='ignore').strip()
                        if line.startswith("N:"):
                            parts = line.split(":")
                            if len(parts) >= 3:
                                mask = int(parts[1])
                                chord = parts[2]
                                self.update_bubbles(mask, chord)
                except Exception:
                    pass
            else:
                threading.Event().wait(1.0)

    def update_bubbles(self, mask, chord_name):
        self.chord_label.configure(text=chord_name)
        for i in range(5):
            is_on = (mask >> i) & 1
            color = self.btn_colors[i] if is_on else "#333333"
            self.note_canvas.itemconfig(self.bubbles[i], fill=color)

    # --- CONNEXIONS MATÉRIELLES ---
    def connect_audio(self):
        try:
            # Cherche la Teensy dans la liste des périphériques audio (entrées)
            devices = sd.query_devices()
            device_idx = None
            for i, dev in enumerate(devices):
                if 'Teensy' in dev['name'] and dev['max_input_channels'] > 0:
                    device_idx = i
                    break
            
            if device_idx is not None:
                # Ouvre le flux audio avec la fonction callback
                self.audio_stream = sd.InputStream(
                    device=device_idx, 
                    channels=1, 
                    samplerate=44100, 
                    blocksize=self.chunk_size, 
                    callback=self.audio_callback
                )
                self.audio_stream.start()
                print(f"Audio Connecté: {devices[device_idx]['name']}")
            else:
                print("Audio: Périphérique Teensy introuvable. Avez-vous mis 'Serial + MIDI + Audio' ?")
        except Exception as e:
            print(f"Erreur Audio: {e}")

    def connect_midi(self):
        try:
            name = next((p for p in mido.get_output_names() if "Teensy" in p), None)
            if name:
                self.midi_out = mido.open_output(name)
                print(f"MIDI Connecté: {name}")
        except: pass

    def connect_serial(self):
        try:
            ports = serial.tools.list_ports.comports()
            t_port = next((p.device for p in ports if "Teensy" in p.description or "Serial" in p.description), None)
            if not t_port and ports: t_port = ports[0].device
            if t_port:
                self.serial_in = serial.Serial(t_port, 9600, timeout=1)
                print(f"Série Connecté: {t_port}")
        except: pass

    def on_closing(self):
        self.running = False
        
        # Fermeture de l'audio
        if self.audio_stream:
            try:
                self.audio_stream.stop()
                self.audio_stream.close()
            except: pass
            
        if self.midi_out: 
            try: self.midi_out.close()
            except: pass
            
        if self.serial_in: 
            try: self.serial_in.close()
            except: pass
            
        plt.close('all')
        
        try:
            self.quit()
            self.destroy()
        except:
            pass
            
        os._exit(0)

if __name__ == "__main__":
    app = TeensyControllerApp()
    app.mainloop()