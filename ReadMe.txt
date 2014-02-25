+项目目录介绍：
	cmake			linux 下cmake编译脚本，要首先安装cmake编译工具
	common			revolver核心框架代码目录
		+revolver	revolver基础封装代码目录
		+core		revolver的框架代码目录
		+json		一个JSON的封装代码目录
		+rudp		revolver的rudp实现代码目录
		+silencer	revolver的服务器扩展代码目录
	daemon			精灵服务器代码目录
	protocol_analysis	def协议翻译程序代码目录（由金山zhurw提供）
	test			测试代码目录（所有相关的模块测试例子、服务器框架例子等）

编译说明:
1.windows下
	用VS 2010 打开common的工程，编译revolver的核心代码，让后可以在各个目录下打开相对应的工程进行编译，如果要使用到协议翻译器，请打开protocol_analysis目录下的工程进行编译，执行程序会生成在common/def目录下
2.linux下
	先安装cmake编译器，在centos下用yum install cmake,在ubuntu下apt-get install cmake
	切换到revolver/cmake目录，输入cmake .
	完成后，make就可以生成各个执行文件，执行文件在revolver/cmake/bin下

需要了解关于revolver细节，请访问博客http://blog.csdn.net/u012935160