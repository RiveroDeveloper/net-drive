import customtkinter as ctk
from PIL import Image
import car as car_module


# CONFIGURACIÓN GLOBAL
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# CREAR VENTANA PRINCIPAL
root = ctk.CTk()
root.title("Telemetry Client")
root.geometry("800x600")
root.resizable(False, False)

# FRAME PRINCIPAL
main_frame = ctk.CTkFrame(root)
main_frame.pack(fill="both", expand=True)

# HEADER
header_frame = ctk.CTkFrame(main_frame, fg_color="#2C2C2C", corner_radius=0)
header_frame.pack(fill="x", pady=0, padx=0)

title_label = ctk.CTkLabel(
    header_frame, 
    text="NetDrive", 
    font=("Verdana", 24, "bold"),
    text_color="white"
)
title_label.pack(pady=20)

# CONTENIDO PRINCIPAL
content_frame = ctk.CTkFrame(main_frame, fg_color="gray10", corner_radius=0)
content_frame.pack(fill="both", expand=True)

# CONFIGURAR GRID PRINCIPAL (3 columnas)
content_frame.grid_columnconfigure(0, weight=1)  # Grid izquierdo
content_frame.grid_columnconfigure(1, weight=1)  # Imagen central
content_frame.grid_columnconfigure(2, weight=1)  # Grid derecho
content_frame.grid_rowconfigure(0, weight=1)     # Fila única

# GRID IZQUIERDO (Velocidad y Batería)
left_grid = ctk.CTkFrame(content_frame, fg_color="transparent")
left_grid.grid(row=0, column=0, padx=(5, 15), pady=20, sticky="nsew")

# Configurar grid izquierdo (2 filas)
left_grid.grid_rowconfigure(0, weight=1)
left_grid.grid_rowconfigure(1, weight=1)
left_grid.grid_columnconfigure(0, weight=1)

# VELOCIDAD (arriba izquierda)
speed_frame = ctk.CTkFrame(
    left_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
speed_frame.grid(row=0, column=0, padx=5, pady=5, sticky="nsew")

speed_title = ctk.CTkLabel(
    speed_frame,
    text="VELOCIDAD",
    font=ctk.CTkFont(size=16, weight="bold"),
    text_color="white"
)
speed_title.pack(pady=(15, 5))

speed_value = ctk.CTkLabel(
    speed_frame,
    text=car_module.car.speed,
    font=ctk.CTkFont(size=36, weight="bold"),
    text_color="white"
)
speed_value.pack()

speed_unit = ctk.CTkLabel(
    speed_frame,
    text="km/h",
    font=ctk.CTkFont(size=14),
    text_color="white"
)
speed_unit.pack(pady=(0, 15))

# BATERÍA (abajo izquierda)
battery_frame = ctk.CTkFrame(
    left_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
battery_frame.grid(row=1, column=0, padx=5, pady=5, sticky="nsew")

battery_title = ctk.CTkLabel(
    battery_frame,
    text="BATERÍA",
    font=ctk.CTkFont(size=16, weight="bold"),
    text_color="white"
)
battery_title.pack(pady=(15, 5))

battery_value = ctk.CTkLabel(
    battery_frame,
    text=car_module.car.battery,
    font=ctk.CTkFont(size=36, weight="bold"),
    text_color="white"
)
battery_value.pack()

battery_unit = ctk.CTkLabel(
    battery_frame,
    text="%",
    font=ctk.CTkFont(size=14),
    text_color="white"
)
battery_unit.pack(pady=(0, 15))

# FRAME CENTRAL PARA IMAGEN DEL CARRO
car_frame = ctk.CTkFrame(
    content_frame,
    corner_radius=15,
    fg_color="transparent"
)
car_frame.grid(row=0, column=1, padx=5, pady=20, sticky="nsew")

# Cargar imagen del carro
def load_car_image():
    try:
        car_image = Image.open("images/auto.png")
        
        # Usar CTkImage en lugar de ImageTk.PhotoImage
        car_photo = ctk.CTkImage(
            light_image=car_image,
            dark_image=car_image,
            size=(400, 500)
        )
        
        car_image_label = ctk.CTkLabel(
            car_frame,
            image=car_photo,
            text=""
        )
        car_image_label.pack(expand=True, padx=20, pady=20)
        car_image_label.image = car_photo  # Mantener referencia
    except Exception as e:
        # Fallback si no se encuentra la imagen
        car_image_label = ctk.CTkLabel(
            car_frame,
            text="IMAGEN DEL CARRO",
            font=ctk.CTkFont(size=20),
            text_color="white"
        )
        car_image_label.pack(expand=True)
        
def update_telemetry(speed, temp, direction, battery):
    speed_value.configure(text=str(speed))
    temp_value.configure(text=str(temp))
    direction_value.configure(text=direction)
    battery_value.configure(text=str(battery))

# Cargar la imagen
load_car_image()

# GRID DERECHO (Temperatura y Dirección)
right_grid = ctk.CTkFrame(content_frame, fg_color="transparent")
right_grid.grid(row=0, column=2, padx=(0, 5), pady=20, sticky="nsew")

# Configurar grid derecho (2 filas)
right_grid.grid_rowconfigure(0, weight=1)
right_grid.grid_rowconfigure(1, weight=1)
right_grid.grid_columnconfigure(0, weight=1)

# TEMPERATURA (arriba derecha)
temp_frame = ctk.CTkFrame(
    right_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
temp_frame.grid(row=0, column=0, padx=5, pady=5, sticky="nsew")

temp_title = ctk.CTkLabel(
    temp_frame,
    text="TEMPERATURA",
    font=ctk.CTkFont(size=16, weight="bold"),
    text_color="white"
)
temp_title.pack(pady=(15, 5))

temp_value = ctk.CTkLabel(
    temp_frame,
    text=car_module.car.temp,
    font=ctk.CTkFont(size=36, weight="bold"),
    text_color="white"
)
temp_value.pack()

temp_unit = ctk.CTkLabel(
    temp_frame,
    text="°C",
    font=ctk.CTkFont(size=14),
    text_color="white"
)
temp_unit.pack(pady=(0, 15))

# DIRECCIÓN (abajo derecha)
direction_frame = ctk.CTkFrame(
    right_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
direction_frame.grid(row=1, column=0, padx=5, pady=5, sticky="nsew")

direction_title = ctk.CTkLabel(
    direction_frame,
    text="DIRECCIÓN",
    font=ctk.CTkFont(size=16, weight="bold"),
    text_color="white"
)
direction_title.pack(pady=(15, 5))

direction_value = ctk.CTkLabel(
    direction_frame,
    text=car_module.car.direction,
    font=ctk.CTkFont(size=28, weight="bold"),
    text_color="white"
)
direction_value.pack()

direction_icon = ctk.CTkLabel(
    direction_frame,
    text="°",
    font=ctk.CTkFont(size=20),
    text_color="white"
)
direction_icon.pack(pady=(0, 15))

#root.mainloop()