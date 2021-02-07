#include<iostream>
using namespace std;

typedef class Node
{
public:
	int data;
	class Node *next;
}ListNode;

class List
{
public:
	//构建单链表并初始化(头插法)
/*	List(int a[], int n){
		ListNode *s;
		int i;
		L = (ListNode *)malloc(sizeof(ListNode));
		L->next = NULL;
		for (i = 0; i < n; i++){
			s = (ListNode *)malloc(sizeof(ListNode));
			s->data = a[i];
			s->next = L->next;
			L->next = s;
		}
	}*/
	//构建并初始化（尾插法）
	List(int a[], int n) {
		ListNode *p, *s;
		int i;
		L = (ListNode *)malloc(sizeof(ListNode));
		p = L;
		for (i = 0; i < n; i++) {
			s = (ListNode *)malloc(sizeof(ListNode));
			s->data = a[i];
			p->next = s;
			p = s;
		}
		p->next = NULL;
	}
	//析构，释放空间
	~List() {
		ListNode *pre = L, *p = L->next;
		while (p->next != NULL) {
			free(pre);
			pre = p;
			p = pre->next;
		}
		free(pre);
	}
	//判断是否为空
	bool isEmpty() {
		return (L->next == NULL);
	}
	//单链表长度
	int getLength() {
		ListNode *p = L;
		int i = 0;
		while (p->next != NULL) {
			i++;
			p = p->next;
		}
		return i;
	}
	//输出单链表中元素
	void DisplayList() {
		ListNode *p = L->next;//指向第一个存有数据的节点（非头结点）
		while (p != NULL) {
			cout << p->data << ends;
			p = p->next;
		}
		cout << endl;
	}
	//按序号查找并返回数据值
	bool getElem(int i, int &res) {
		ListNode *p = L;
		int j = 0;
		while (j != i && p->next != NULL) {
			j++;
			p = p->next;
		}
		if (j != i && p->next == NULL) {//必须还有j!=i，否则最后一个节点无法访问
			return false;
		}
		else {
			res = p->data;
			return true;
		}
	}
	//按值查找并返回序号
	int LocateElem(int &elem) {
		ListNode *p = L;
		int i = 0;
		while (p->data != elem && p->next != NULL) {
			i++;
			p = p->next;
		}
		if (p->data != elem && p->next == NULL) {
			return 0;
		}
		else {
			return i;
		}
	}
	//插入元素
	void InsertElem(int i, int &elem) {
		i--;
		ListNode *p = L, *s;
		int j = 0;
		while (j != i && p->next != NULL) {
			j++;
			p = p->next;
		}
		if (p->next == NULL) {
			cout << "error number." << endl;
		}
		else {
			s = (ListNode *)malloc(sizeof(ListNode));
			s->data = elem;
			s->next = p->next;
			p->next = s;
		}
	}
	//删除元素
	bool DeleteElem(int i, int &res) {
		i--;
		ListNode *p = L, *s;
		int j = 0;
		while (j != i && p->next != NULL) {
			j++;
			p = p->next;
		}
		if (p->next == NULL) {
			return false;
		}
		else {
			s = p->next;
			/*	if (s->next == NULL)
					return false;*/
			res = s->data;
			p->next = p->next->next;
			free(s);
			return true;
		}
	}
private:
	Node *L;
	//返回头结点地址
};

//int main()
//{
//	//ListNode *m_node;
//	int n, res = 0, a[100], i;
//	cout << "Input the length:" << ends;
//	cin >> n;
//	for (i = 0; i < n; i++) {
//		cin >> a[i];
//	}
//	//创建单链表
//	List m_list(a, n);
//	//输出
//	m_list.DisplayList();
//	//判断空,非空返回长度
//	if (!m_list.isEmpty()) {
//		cout << "it is not empty." << endl;
//		cout << "The length:" << m_list.getLength() << endl;
//	}
//	else {
//		cout << "It is empty." << endl;
//	}
//	//按序号查找并返回元素
//	cout << "Input number:" << ends;
//	cin >> i;
//	if (m_list.getElem(i, res)) {
//		cout << "NO. " << i << " ,data is " << res << endl;
//	}
//	else {
//		cout << "error number or not contained." << endl;
//	}
//	//按值查找并返回序号
//	cout << "Input the data:" << ends;
//	cin >> res;
//	if (m_list.LocateElem(res) == 0) {
//		cout << "Not contained." << endl;
//	}
//	else {
//		cout << "The location is No. " << m_list.LocateElem(res) << endl;
//	}
//	//插入元素
//	cout << "Input the number and data:" << ends;
//	cin >> i >> res;
//	m_list.InsertElem(i, res);
//	m_list.DisplayList();
//	//删除元素
//	cout << "Input number:" << ends;
//	cin >> i;
//	if (m_list.DeleteElem(i, res)) {
//		cout << "The NO. " << i << " ,data: " << res << " is deleted." << endl;
//		m_list.DisplayList();
//	}
//	else {
//		cout << "Not contain this number." << endl;
//	}
//	return 0;
//}