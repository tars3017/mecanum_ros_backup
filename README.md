# mecanum_ros_backup

## 使用說明
1. 連線上樹莓派(vscode要安裝ssh的plugin, 密碼在雲端硬碟)
2. 進到 cd ~/mecanum_ws/
3. 在terminal輸入```source devel/setup.bash```
4. 接著輸入```roslaunch mecanum_steady commu.launch```開啟與 STM 的通訊
5. 開一個新的 terminal，一樣進入cd ~/mecanum_ws/，輸入```roslaunch mecanum_steady run.luanch```，裡面包含
  控制速度的程式以及設定目標點的程式(可選擇 auto mode 或是手動輸入每個點)
7. 輸入目標點，(x, y, z)三個數字，請不要輸入現在的座標，座標要不同車子才會動
8. 每次測試完都要用 ctrl + C 和 ctrl + D 關掉程式(需關掉 move.launch 跟 main_node)

---
Technique
Use constant acceleration and simple p-control to send the velocity to STM.
Expected velocity look like this: 
![image](https://github.com/tars3017/mecanum_ros_backup/assets/64685394/6eb198d2-9911-427e-898c-f830dbf5962e)

---
Final Result

https://github.com/tars3017/mecanum_ros_backup/assets/64685394/f3f76957-f3d2-4d47-8221-bf9d21da3d03

