import cv2
import numpy as np


def apply_tint(rgb_img, color):
    # gray -> BGR
    gray_img = cv2.cvtColor(rgb_img, cv2.COLOR_BGR2GRAY)

    # normalize
    normalized = gray_img / 255.0

    print(normalized.shape)
    print((normalized[..., None]).shape)
    print(normalized[0])
    print(normalized[..., None][0])

    # Scale the color by the normalized intensity
    tinted = (color * normalized[..., None]).astype(np.uint8)

    print(rgb_img.size, rgb_img.shape)
    print(gray_img.size, gray_img.shape)
    print(gray_img.size, gray_img.shape)
    print(tinted.size, tinted.shape)

    # blend them
    # result = cv2.addWeighted(
    #     rgb_img,
    #     1 - normalized,
    #     tinted,
    #     normalized,
    #     0
    # )
    result = cv2.addWeighted(
        rgb_img,
        0,
        tinted,
        1,
        0
    )

    return result


# Load grayscale image
# img_path = input('Image path: ')
img_path = r'D:\Manbir\Faithful 32x Java\assets\minecraft\textures\block\fern.png'

img_gray = cv2.imread(img_path)
tint_color_1 = (80, 91, 84)
tint_color_2 = (84, 91, 80)

img_tinted_1 = apply_tint(img_gray, np.array(tint_color_1))
img_tinted_2 = apply_tint(img_gray, np.array(tint_color_2))

# Display the result
cv2.imshow(
    'Original',
    cv2.resize(img_gray, (500, 500), interpolation=cv2.INTER_NEAREST)
)
cv2.imshow(
    'Tinted 1',
    cv2.resize(img_tinted_1, (500, 500), interpolation=cv2.INTER_NEAREST)
)
cv2.imshow(
    'Tinted 2',
    cv2.resize(img_tinted_2, (500, 500), interpolation=cv2.INTER_NEAREST)
)

cv2.waitKey(0)
cv2.destroyAllWindows()
