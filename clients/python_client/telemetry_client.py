import customtkinter as ctk
from PIL import Image
import car as car_module


# Global appearance
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# Main window
root = ctk.CTk()
root.title("Telemetry Client")
root.geometry("800x600")
root.resizable(False, False)

# Main frame
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

# Content area
content_frame = ctk.CTkFrame(main_frame, fg_color="gray10", corner_radius=0)
content_frame.pack(fill="both", expand=True)

# 3-column grid
content_frame.grid_columnconfigure(0, weight=1)
content_frame.grid_columnconfigure(1, weight=1)
content_frame.grid_columnconfigure(2, weight=1)
content_frame.grid_rowconfigure(0, weight=1)

# Left: speed + battery
left_grid = ctk.CTkFrame(content_frame, fg_color="transparent")
left_grid.grid(row=0, column=0, padx=(5, 15), pady=20, sticky="nsew")

# Left grid — 2 rows
left_grid.grid_rowconfigure(0, weight=1)
left_grid.grid_rowconfigure(1, weight=1)
left_grid.grid_columnconfigure(0, weight=1)

# Speed (top-left)
speed_frame = ctk.CTkFrame(
    left_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
speed_frame.grid(row=0, column=0, padx=5, pady=5, sticky="nsew")

speed_title = ctk.CTkLabel(
    speed_frame,
    text="SPEED",
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

# Battery (bottom-left)
battery_frame = ctk.CTkFrame(
    left_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
battery_frame.grid(row=1, column=0, padx=5, pady=5, sticky="nsew")

battery_title = ctk.CTkLabel(
    battery_frame,
    text="BATTERY",
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

# Center: vehicle image
car_frame = ctk.CTkFrame(
    content_frame,
    corner_radius=15,
    fg_color="transparent"
)
car_frame.grid(row=0, column=1, padx=5, pady=20, sticky="nsew")

# Load vehicle image
def load_car_image():
    try:
        car_image = Image.open("images/auto.png")
        
        # CTkImage
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
        car_image_label.image = car_photo
    except Exception as e:
        car_image_label = ctk.CTkLabel(
            car_frame,
            text="Vehicle image",
            font=ctk.CTkFont(size=20),
            text_color="white"
        )
        car_image_label.pack(expand=True)
        
def update_telemetry(speed, temp, direction, battery):
    speed_value.configure(text=str(speed))
    temp_value.configure(text=str(temp))
    direction_value.configure(text=direction)
    battery_value.configure(text=str(battery))

# Image loaded above
load_car_image()

# Right: temperature + heading
right_grid = ctk.CTkFrame(content_frame, fg_color="transparent")
right_grid.grid(row=0, column=2, padx=(0, 5), pady=20, sticky="nsew")

# Right grid — 2 rows
right_grid.grid_rowconfigure(0, weight=1)
right_grid.grid_rowconfigure(1, weight=1)
right_grid.grid_columnconfigure(0, weight=1)

# Temperature (top-right)
temp_frame = ctk.CTkFrame(
    right_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
temp_frame.grid(row=0, column=0, padx=5, pady=5, sticky="nsew")

temp_title = ctk.CTkLabel(
    temp_frame,
    text="TEMPERATURE",
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

# Heading (bottom-right)
direction_frame = ctk.CTkFrame(
    right_grid,
    corner_radius=10,
    fg_color="#2C2C2C"
)
direction_frame.grid(row=1, column=0, padx=5, pady=5, sticky="nsew")

direction_title = ctk.CTkLabel(
    direction_frame,
    text="HEADING",
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