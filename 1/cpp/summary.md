4.18文件实现内容：

- 帧结构定义
- 读取文件并发送/接受文件并保存
- 部分全双工功能

4.18遇到的问题：

- 先运行A，后运行B时，A接收不到B发送的内容

接下来要实现的内容（可以再商量）：

- 将数据处理成帧/将帧处理成数据——赵志鹏
- 完成全双工功能——桂梦婷
- 做GO-BACK-N算法的前置准备（计时器等），以及配置文件的书写——张子阳



4.28文件实现内容：

- 帧处理（封装和解包检查错误）

4.29新设想：

- 要实现双向同时传输，遇到了ack包和seq包区分的问题
- （解决）为接收和发送分别创建一个“缓冲区”（循环队列），不管拿到什么包，直接往缓冲区扔[当然，这里需要保证<u>互斥访问</u>]，处理线程负责处理（分别为send和recv建立处理线程，或者建立1个），处理线程不仅负责处理数据，还负责填写文件等等

4.29需要实现内容:

- Go-Back-N算法计时器设计，窗口设计（zz阳）
  - GO-BACK-N需要有一个n个大小的窗口，接收方有1个窗口
  - GO-BACK-N的发送方要维持1个计时器（最早发送且未收到ack信息的那个窗口），需要有个n帧大小的滑动窗口
  - （之后）考虑接收方收到错误数据或者超时未接收后，是选择等待超时重传还是直接返回错误信息{如果采用方式2，需要考虑帧的结构}
  - （之后）考虑如何实现“数据错误”“数据丢失”

- 完成处理线程的处理逻辑（桂），*关于返回需要发送什么，需要后台再处理什么，这是GBN的工作*
- 建立可互斥访问的队列及移动指针（桂）

存疑：

- 将函数封装到frame里，可行吗？
- 建立返回帧和发送帧的逻辑是否正确，199line

5.2实现了：

- 封装帧和解开帧，建立帧发送队列和接收队列
- 处理线程对帧的解释

问题

- 发送时会多接收一帧
- toData函数总是返回错误值

要完成：

- GBN算法
- 配置文件和输出文件的书写



截止至5.14的进度：

已完成：

- 5.2的配置文件

未完成：

- GBN算法
- 输出文件（正在进行）





5.20进度

- 部分合了GBN算法

5.20问题

- **（严重问题！！优先解决！）**接收到的帧仍无法解读出length，其他内容都可以(zhao & gui)
- 199行：winsize的定义不清晰，已用todo标出(zhang)，为了程序运行我暂时修改了一下，实际逻辑要弄清楚
- 213行：发送错误帧格式无定义（zhao），与此对应的处理逻辑
- 定时器和超时设计是否要用循环(zhang & gui) ，只用循环时间会不会比较短，开线程(gui)
- 237行：窗口和写入文件的逻辑需要弄明白(zhang)，假设大小为5的接收窗口，什么时候才能写入文件
- 写入文件时的输出配置(gui)

仍未解决的问题：

- 超时重传和主动发送重传请求，后者的帧格式是什么(zhao & gui)
- 初始化配置嵌入到程序中(gui)
- 错误传输的处理（比如修改数据），超时传输如何实现(??)
- 输出格式(gui)



5.27进度

- 将初始化配置嵌入程序(gui)，错误和丢失已嵌入，剩datasize没有嵌入
- 将GBN嵌入程序(gui)
- 输出格式完成(gui)
- 问题2，问题5已解决(gui)

仍未解决的问题：

- 定时器如何嵌入？GBN算法的细节，即设置几个定时器？zhang
- 配置文件中只给出了一个UDPPort，但作为全双工来说既需要sendport又需要recvport (zhao)
- **（严重问题！！优先解决！）**接收到的帧仍无法解读出length，其他内容都可以(zhao & gui)

有想法很快就能解决的问题：

- 开线程计时
- (gui)sendq队列应该就当作gbn的窗口进行滑动，不需要额外的空间,这里需要修改



6.1进度

- 基本完成GBN

问题：

- 需要在最开始的连接帧就发出seq末尾的那个号，方便结束接收等等
- 配置读取的内容
- 发生错误和重传尚未测试



6.3进度

- 无差错传输完成

问题：

- 配置A
- 结束进程（想法：接收到结尾ACK后，将endACK置1结束deal进程，将endFrameR/S置1结束其他两个进程
- 发生错误尚未解决



6.5

- 修改成全双工
- 配置完成
- 有差错/丢失/无差错均完成

问题：

- 有一个程序无法在接收结束后结束，但它获取的数组是正确的