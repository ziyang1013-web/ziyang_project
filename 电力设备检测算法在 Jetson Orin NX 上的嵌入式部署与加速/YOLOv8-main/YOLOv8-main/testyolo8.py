# 在你的 PC 上测试模型能否正常加载
from ultralytics import YOLO

# 用你训练好的 best.pt 测试
model = YOLO('D:\deeeplearn\YOLOv8-main\runs\detect\train8\weights/best.pt')
print("模型加载成功！")

# 用一张测试图片验证推理
results = model('D:\deeeplearn\high_wire_data\wiredata\test\images/1-17-_jpg.rf.06dfe9aee0263f4afd017d247b53bd98.jpg')
results[0].save('test_output.jpg')
print("推理成功，结果已保存")