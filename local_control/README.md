# ESP Local Control Client

Script Python để kết nối và điều khiển ESP32 device qua ESP Local Control Protocol.

## Tính năng

- ✅ Kết nối với ESP32 device qua mDNS
- ✅ Liệt kê các properties có sẵn
- ✅ Giao diện tương tác để thay đổi giá trị properties
- ✅ Hỗ trợ HTTPS với self-signed certificates
- ✅ Chế độ non-interactive cho automation

## Cài đặt

1. Cài đặt dependencies:
```bash
pip install -r requirements.txt
```

2. Đảm bảo ESP32 device đang chạy và kết nối WiFi với tên service `my_esp_ctrl_device.local`

## Cách sử dụng

### Chế độ Interactive (như trong hình ảnh)

```bash
python esp_local_ctrl.py --sec_ver 0
```

Script sẽ:
1. Kết nối với `my_esp_ctrl_device.local`
2. Hiển thị "Starting Session" và "Session Established"
3. Liệt kê Available Properties trong bảng
4. Cho phép chọn property và nhập giá trị mới
5. Hiển thị properties đã cập nhật

### Chế độ Non-interactive

```bash
# Thiết lập property status thành false
python esp_local_ctrl.py --sec_ver 0 --property status --value '{"status": false}'
```

### Các tham số

- `--sec_ver`: Security version (mặc định: 0 cho PROTOCOM_SEC0)
- `--service_name`: Tên mDNS service (mặc định: my_esp_ctrl_device)
- `--property`: Tên property cần thiết lập (chế độ non-interactive)
- `--value`: Giá trị mới cho property (chế độ non-interactive)

## Ví dụ Output

```
Connecting to my_esp_ctrl_device.local
==== Starting Session ====
==== Session Established ====
==== Available Properties ====
S.N.   Name                 Type       Flags    Value
--------------------------------------------------------------------------------
[1]    status               STRING              {"status": true}

Select properties to set (0 to re-read, 'q' to quit): 1
Enter value to set for property (status): {"status": false}
Property 'status' updated successfully!

==== Available Properties ====
S.N.   Name                 Type       Flags    Value
--------------------------------------------------------------------------------
[1]    status               STRING              {"status": false}
```

## Khắc phục lỗi mDNS trên Windows

### Vấn đề: "Could not resolve my_esp_ctrl_device.local"

Windows không hỗ trợ mDNS mặc định. Có 3 cách giải quyết:

#### Cách 1: Cài đặt Bonjour Print Services
1. Tải và cài đặt [Bonjour Print Services for Windows](https://support.apple.com/kb/DL999)
2. Restart máy tính
3. Chạy lại script

#### Cách 2: Sử dụng IP trực tiếp
```bash
# Tìm IP của ESP32 trong router admin hoặc serial monitor
python esp_local_ctrl.py --ip 192.168.1.100 --sec_ver 0
```

#### Cách 3: Để script tự scan network
Script sẽ tự động scan local network nếu không tìm thấy mDNS.

## Lưu ý

- Script tự động xử lý self-signed certificates
- Nếu không tìm thấy certificate file, sẽ sử dụng unverified SSL
- Hỗ trợ fallback properties nếu ESP device không phản hồi đúng format
- Tương thích với ESP32 Local Control Protocol implementation
- Tự động fallback cho Windows mDNS issues
