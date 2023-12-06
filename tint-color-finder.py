import cv2
import numpy as np


def transparent_to_white(image: cv2.Mat | np.ndarray) -> cv2.Mat:

    # Create a mask of the transparent pixels
    mask = np.zeros(image.shape[:2], dtype=np.uint8)
    for i in range(image.shape[0]):
        for j in range(image.shape[1]):
            if image[i, j][3] == 0:
                mask[i, j] = 255

    # Expand the mask to have four channels
    mask_expanded = cv2.cvtColor(mask, cv2.COLOR_GRAY2RGBA)

    # Invert the mask
    inverted_mask = cv2.bitwise_not(mask_expanded)

    # Create a mask of the non-transparent pixels
    non_transparent_mask = cv2.bitwise_and(np.ones_like(image), inverted_mask)

    # Create a destination image for the BGR conversion
    # Initialize with black background
    bgr_image = np.zeros_like(image)[:, :, ::-1]

    # Copy the RGB values from the original RGBA image to the transparent pixels in the BGR image
    cv2.copyTo(image, non_transparent_mask, bgr_image)

    # Convert the image to BGR color space without losing the transparency
    bgr_image = cv2.cvtColor(bgr_image, cv2.COLOR_RGBA2BGR)

    return bgr_image


def find_tint_color(grayscale_img, tinted_img):
    # Ensure the images have the same shape
    assert grayscale_img.shape == tinted_img.shape, "Images must have the same shape"

    if grayscale_img.shape[2] == 4:
        grayscale_img = transparent_to_white(grayscale_img)
        tinted_img = transparent_to_white(tinted_img)

    cv2.imshow("Grayscale", grayscale_img)
    cv2.imshow("Tinted", tinted_img)

    # Calculate color difference
    color_difference = tinted_img - grayscale_img

    # Average the color differences
    average_color_difference = np.mean(color_difference, axis=(0, 1))

    # Convert the average color difference to uint8 to get the tint color
    tint_color = np.round(average_color_difference).astype(np.uint8)

    return tint_color


if __name__ == '__main__':
    # gray_img_path = input('Grayscale image path: ')
    # tinted_img_path = input('Tinted image path: ')

    gray_img_path = r'C:\Users\manbi\Downloads\Untitled (1).png'
    tinted_img_path = r'C:\Users\manbi\Downloads\Untitled.png'

    grayscale_image = cv2.imread(gray_img_path, cv2.IMREAD_COLOR)
    tinted_image = cv2.imread(tinted_img_path, cv2.IMREAD_COLOR)

    tint_color_rgb = tuple(find_tint_color(grayscale_image, tinted_image))
    tint_color_hex = '#%02x%02x%02x' % tint_color_rgb

    print(tint_color_rgb)
    print(tint_color_hex)

    cv2.waitKey(0)
    cv2.destroyAllWindows()
