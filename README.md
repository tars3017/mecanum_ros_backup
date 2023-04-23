# mecanum_ros_backup

## 使用說明
1. 連線上樹莓派(vscode要安裝ssh的plugin, 密碼在雲端硬碟)
2. 進到 ~/mecanum_ws/
3. 在terminal輸入```source devel/setup.bash```
4. 接著輸入```roslaunch mecanum_steady commu.launch```開啟與 STM 的通訊
5. 開一個新的 terminal，一樣進入~/mecanum_ws/，輸入```roslaunch mecanum_steady move.launch```，
此為控制車子速度的程式
6. 再開一個新的 terminal，進入~/mecanum_ws，輸入```rosrun mecanum_steady main_node```，此為輸入目標點座標的程式
7. 輸入目標點，(x, y, z)三個數字，請不要輸入現在的座標，座標要不同車子才會動
8. 每次測試完都要用 ctrl + C 和 ctrl + D 關掉程式(需關掉 move.launch 跟 main_node)
