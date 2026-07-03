# detect_usb_cpu.py - 亚博Jetson Orin NX USB摄像头部署代码（CPU版本）

import cv2
from ultralytics import YOLO
import time
import os

def find_usb_camera():
    """自动查找USB摄像头设备号"""
    for i in [1, 0]:  # 先检查 video1（亚博USB通常在此），再检查 video0
        device_path = f'/dev/video{i}'
        if os.path.exists(device_path):
            cap = cv2.VideoCapture(i)
            if cap.isOpened():
                cap.release()
                print(f"找到可用摄像头: /dev/video{i}")
                return i
    return 0

def main():
    # 加载模型（使用 .pt 文件，不是 .engine）
    # 根据你的实际路径修改
    model_path = '/home/jetson/Desktop/AAAJ/best.pt'  # 改成你的 best.pt 实际路径
    print(f"正在加载模型: {model_path}")
    model = YOLO(model_path)
    print("模型加载成功！")

    # 自动查找摄像头设备号
    camera_id = find_usb_camera()
    cap = cv2.VideoCapture(camera_id)

    # 设置分辨率
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    if not cap.isOpened():
        print("错误：无法打开摄像头")
        print("请检查：")
        print("1. USB 摄像头是否已插入")
        print("2. 执行 ls /dev/video* 查看设备")
        return

    print("开始检测，按 'q' 退出...")

    # FPS 计算
    fps = 0
    frame_count = 0
    start_time = time.time()

    while True:
        ret, frame = cap.read()
        if not ret:
            print("无法获取视频帧")
            break

        # 推理（指定使用 CPU）
        results = model(frame, conf=0.5, verbose=False, device='cpu')

        # 绘制结果（会自动画出方框和标签）
        annotated_frame = results[0].plot()

        # 计算 FPS
        frame_count += 1
        if frame_count >= 30:
            end_time = time.time()
            fps = frame_count / (end_time - start_time)
            frame_count = 0
            start_time = end_time

        # 显示 FPS
        cv2.putText(annotated_frame, f"FPS: {fps:.1f}",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        # 显示画面
        cv2.imshow("Power Equipment Detection", annotated_frame)

        # 按 'q' 退出
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
    print("程序已退出")

if __name__ == "__main__":
    main()