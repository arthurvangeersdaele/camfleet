import os
from PIL import Image, ImageDraw, ImageFont
from datetime import datetime
import cv2
import numpy as np


# --- Parameters ---
INPUT_FOLDER = "ESP32-CAM_PrusaXL"  # Change this
BLACK_BAND_HEIGHT = 80
FONT_PATH = "C:/Windows/Fonts/arial.ttf" # Update if needed
FONT_SIZE_MAIN = 24
FONT_SIZE_SUB = 16

# Prepare video writer
video_path = os.path.join(INPUT_FOLDER, INPUT_FOLDER .split('_')[-1]+"_video.mp4")
fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # Codec
video_writer = None  # Will initialize after getting first frame size
frame_rate = 3  # 1 frame per second (adjust if needed)

# Process Images ---
for filename in sorted(os.listdir(INPUT_FOLDER)):
    if filename.lower().endswith(('.jpg', '.jpeg')) and 'annotated' not in filename:
        try:
            # Extract parts from filename
            name_parts = filename.split('_')
            date_str = name_parts[0]
            cam_name = name_parts[-1].split('.')[0]

            # Format datetime
            dt = datetime.strptime(date_str, "%Y%m%d-%H%M%S")
            formatted_date = dt.strftime("%d/%m/%Y - %H:%M:%S")

            # Load image
            img_path = os.path.join(INPUT_FOLDER, filename)
            img = Image.open(img_path).convert("RGB")
            width, height = img.size

            # Create new image with black band
            new_height = height + BLACK_BAND_HEIGHT
            new_img = Image.new("RGB", (width, new_height), "black")
            new_img.paste(img, (0, 0))

            draw = ImageDraw.Draw(new_img)
            font_main = ImageFont.truetype(FONT_PATH, FONT_SIZE_MAIN)
            font_sub = ImageFont.truetype(FONT_PATH, FONT_SIZE_SUB)

            # Main and sub text
            margin = 10
            main_text = f"{cam_name}\n{formatted_date}"
            sub_text = "OS 3DP Camera Fleet - Developed by Arthur VG DG"

            draw.multiline_text((margin, height + 5), main_text, fill="white", font=font_main, spacing=4)

            sub_text_bbox = draw.textbbox((0, 0), sub_text, font=font_sub)
            sub_text_width = sub_text_bbox[2] - sub_text_bbox[0]
            sub_text_height = sub_text_bbox[3] - sub_text_bbox[1]

            draw.text(
                (width - sub_text_width - margin, new_height - sub_text_height - 5),
                sub_text,
                fill="white",
                font=font_sub
            )

            # Convert PIL image to OpenCV (BGR format)
            frame = cv2.cvtColor(np.array(new_img), cv2.COLOR_RGB2BGR)

            # Initialize video writer after getting frame size
            if video_writer is None:
                video_writer = cv2.VideoWriter(video_path, fourcc, frame_rate, (width, new_height))

            video_writer.write(frame)
            os.remove(img_path)

        except Exception as e:
            print(f"❌ Error processing {filename}: {e}")

# Finalize
if video_writer:
    video_writer.release()
    print(f"\n🎬 Video saved to: {video_path}")

