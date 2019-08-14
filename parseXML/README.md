# C++ parse XML file

### xml中的变量含义：

==XmlDoc== 包含由解析文档建立的树结构。

==xmlDocPtr== 是指向这个结构的指针。

==xmlNodePtr== and ==xmlNode== 包含单一结点的结构

==xmlNodePtr== 是指向这个结构的指针，它被用于遍历文档树。
###  解析XML文档
#### XML解析流程

解析一个XML文档，从中取出想要的信息，例如节点中包含的文字，或者某个节点的属性。其流程如下：

①    用xmlReadFile函数读入一个文件，并返回一个文档指针doc。

②    用xmlDocGetRootElement函数得到根节点curNode。

③    此时curNode->xmlChildrenNode就是根节点的首个儿子节点，该儿子节点的兄弟节点可用next指针进行轮询。

④    轮询所有子节点，找到所需的节点，用xmlNodeGetContent取出其内容。

⑤    用xmlHasProp查找含有某个属性的节点，属性列表指针xmlAttrPtr将指向该节点的属性列表。

⑥    取出该节点的属性，用xmlGetProp取出其属性值。

⑦    xmlFreeDoc函数关闭文档指针，并清除本文档中所有节点动态申请的内存。

### 本示例操作
利用linu自带的libxml2
是使用g++编译
g++ main.cpp -o main -lxml2 -I/usr/include/libxml2

运行
./main res.xml