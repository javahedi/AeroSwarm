from PIL import Image, ImageDraw, ImageFont
import os
import glob

def text_to_image(text_frame, frame_id, output_folder, scale_factor=20):
    lines = text_frame.strip().split('\n')
    original_width = len(lines[0])
    original_height = len(lines)
    
    # Calculate scaled dimensions
    scaled_width = original_width * scale_factor
    scaled_height = original_height * scale_factor
    
    # Create a new image at the scaled size
    img = Image.new('RGB', (scaled_width, scaled_height), color='white')
    draw = ImageDraw.Draw(img)
    
    # Define colors for each block
    color_map = {
        '⬜': (255, 255, 255),  # white for free
        '⬛': (0, 0, 0),        # black for obstacle
        '🛸': (0, 255, 0),      # green for drone
        '🎯': (255, 0, 0),      # red for target
        'X': (100, 100, 100)    # gray for visited cells (drawn as *)
    }
    
    # Load a font for the * symbol
    try:
        symbol_font = ImageFont.truetype("arial.ttf", int(scale_factor * 0.8))  # Scale font to fit cell
    except:
        symbol_font = ImageFont.load_default()
    
    # Draw cells
    for y in range(original_height):
        for x in range(original_width):
            char = lines[y][x]
            # Calculate rectangle coordinates
            x1 = x * scale_factor
            y1 = y * scale_factor
            x2 = (x + 1) * scale_factor
            y2 = (y + 1) * scale_factor
            if char == 'X':
                # Draw * symbol for visited cells
                text_x = x1 + (scale_factor / 4)  # Center horizontally
                text_y = y1 + (scale_factor / 8)  # Adjust vertically for better centering
                draw.text((text_x, text_y), 'X', fill=color_map['X'], font=symbol_font)
            else:
                # Draw solid rectangle for other cells
                draw.rectangle([x1, y1, x2, y2], fill=color_map.get(char, (255, 255, 255)))
    
    # Add frame ID text
    try:
        font = ImageFont.truetype("arial.ttf", 16)
    except:
        font = ImageFont.load_default()
    
    draw.text((10, 10), f"Frame: {frame_id.split('_')[-1]}", fill="black", font=font)
    
    img.save(os.path.join(output_folder, f'frame_{frame_id}.png'))

def process_all_frames(input_folder, output_folder):
    os.makedirs(output_folder, exist_ok=True)
    
    frame_files = glob.glob(os.path.join(input_folder, '*.txt'))
    
    for frame_file in sorted(frame_files):
        frame_id = os.path.splitext(os.path.basename(frame_file))[0]
        with open(frame_file, 'r', encoding='utf-8') as f:
            text_frame = f.read()
        text_to_image(text_frame, frame_id, output_folder)

def create_gif(image_folder, output_gif, duration=200):
    image_files = sorted(glob.glob(os.path.join(image_folder, '*.png')))
    
    images = []
    for img_path in image_files:
        try:
            img = Image.open(img_path)
            img = img.convert('RGB')
            images.append(img)
        except Exception as e:
            print(f"Error loading {img_path}: {e}")
    
    if images:
        images[0].save(
            output_gif,
            save_all=True,
            append_images=images[1:],
            duration=duration,
            loop=0
        )
        for img in images:
            img.close()
    else:
        print("No images found to create GIF")

if __name__ == '__main__':
    process_all_frames('build/frames', 'build/frames_images')
    create_gif('build/frames_images', 'build/drone_sim.gif', duration=200)