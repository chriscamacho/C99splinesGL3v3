import numpy as np
from PIL import Image

# Load the original font image
original_font_image = Image.open("original-font.png")
original_font_array = np.array(original_font_image)

# Define dimensions
num_characters = original_font_array.shape[1] // 32
character_height = original_font_array.shape[0]
character_width = 32

# Initialize an empty array to store the font strip
font_strip = np.empty((character_height * num_characters, character_width, 4), dtype=np.uint8)

# Build the font strip
for i in range(num_characters):
    character = original_font_array[:, i * character_width:(i + 1) * character_width]
    ni = (num_characters-i)-1
    font_strip[ni * character_height:(ni + 1) * character_height, :, :] = character

# Convert the font strip array to an image
font_strip_image = Image.fromarray(font_strip)

# Save the font strip image as "font.png"
font_strip_image.save("font.png")  # Update with desired file name and format
