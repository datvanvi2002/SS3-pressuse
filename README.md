ap1705 write README in 18052024
- Sửa lại khai báo chân van 1 là chân 4, van 2 giữ nguyên : chân 5
- Hàm setpoint : thêm serial để có thể lấy từ serial 0 và serial 2. Lỗi nhập settpoint khi nhập setpoint = 0
- Hàm dataSerial sửa biến output thành frequency, và chỉ in kiểu nguyên
- Tạo 1 hàm dừng servor = stop PID
- Hàm control servor : Dựa vào error để chọn tốc độ, khi sai lệch từ 0 - 1 sử dụng pid
( cần test để biết hệ thống có thể tiến tới trạng thái lỗi vào khoảng đó hay không, không thì sẽ bị dao động quanh ngưỡng đó và không thể bật pid). Check lại điều kiện dừng.
-phát tần số sửa thành Timer3.setFrequency(frequency).start(); và frequency = 2 * output; thực tế tần số động cơ nhận được là = ouput, do timer đặt nửa chu kì nên f cần nhân 2.
- setup
- khởi tạo serial 2,
- Thêm
  Timer3.attachInterrupt(togglePin).setFrequency(1).start();
  Timer3.stop(); // dừng tinmerr
  digitalWrite(pin_pulse, LOW);//dừng tránh bị floating
  loop : chỉ chạy test đk động cơ.
