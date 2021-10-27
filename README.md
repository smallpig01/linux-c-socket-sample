# linux-c-socket-sample

linux 平台 c++ socket 傳接範例

## Description

建立一個 host-cam 模擬 webcam, 並將生成模擬影像資料傳送給 client.

## Getting Started

### Dependencies

* linux平台
* g++

### Installing

### Executing program

* 編譯 host_can.cpp, client.cpp
  ```
  g++ host_cam.cpp -o host_cam
  g++ client.cpp -o client
  ```

* 開啟兩個 terminal , 先執行 host_can.cpp, 再執行client.cpp
  * terminal 1
    ```
    ./host_cam
    ```
  * terminal 2
    ```
    ./client
    ```


## Help


## Authors



## Version History

* 0.1
    * Initial Release

## Acknowledgments

[TCP Socket Programming 學習筆記](http://zake7749.github.io/2015/03/17/SocketProgramming/)

[setsockopt設置socket詳細用法 UDP packet drop 處理](https://blog.xuite.net/csiewap/cc/66342249)

[socket為send和recv設定超時時間](https://www.itread01.com/content/1549594633.html)

[how-to-completely-destroy-a-socket-connection-in-c](https://stackoverflow.com/questions/10619952/how-to-completely-destroy-a-socket-connection-in-c)

[TCP連線的TIME_WAIT和CLOSE_WAIT 狀態解說](https://iter01.com/68377.html)
