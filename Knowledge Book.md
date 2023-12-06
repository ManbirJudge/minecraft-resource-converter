# Knowledge Book
## ID Map Format
### Java ID Map Format
### Bedrock ID Map Format
- If value of a key is a string, its the output path.
- If value of a key is an object, it contains -
    - **Output path**
    - Read mode
- Read modes -
    - `0` - RGB (BGR for OpenCV)
    - `1` - RGBA (BGRA for OpenCV)
    - Default - Decided by OpenCV (subject to change to `0` by default)
## Resource pack format
It is an integer value in pack config of Minecraft Java resource pack which relates to the mincraft (java) version  for which the resource pack is made.
### Values
- **1 -** 1.6.1–1.8.9
- **2 -** 1.9–1.10.2
- **3 -** 1.11–1.12.2
- **4 -** 1.13–1.14.4
- **5 -** 1.15–1.16.1
- **6 -** 1.16.2–1.16.5
- **7 -** 1.17–1.17.1
- **8 -** 1.18–1.18.2
- **9 -** 1.19–1.19.2
- **11 -** 22w42a–22w44a
- **12 -** 1.19.3–Now