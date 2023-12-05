import cv2
import numpy as np


def find_tint_color(grayscale_img, tinted_img):
    # Ensure the images have the same shape
    assert grayscale_img.shape == tinted_img.shape, "Images must have the same shape"

    # Calculate color difference
    color_difference = tinted_img - grayscale_img

    # Average the color differences
    average_color_difference = np.mean(color_difference, axis=(0, 1))

    # Convert the average color difference to uint8 to get the tint color
    tint_color = np.round(average_color_difference).astype(np.uint8)

    return tint_color


if __name__ == '__main__':
    grayscale_image = cv2.imread(input('Grayscale image path: '))
    tinted_image = cv2.imread(input('Tinted image path: '))

    tint_color_rgb = tuple(find_tint_color(grayscale_image, tinted_image))
    tint_color_hex = '#%02x%02x%02x' % tint_color_rgb

    print(tint_color_rgb)
    print(tint_color_hex)
