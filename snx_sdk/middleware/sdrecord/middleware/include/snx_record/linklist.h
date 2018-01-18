#ifndef  __LINK_LIST_H__
#define __LINK_LIST_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavdevice/avdevice.h>
#include <libavutil/parseutils.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>




#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


typedef struct LNode {
	AVPacket *pkt;
	int read_mark;
	struct LNode *next;
}LNode, *pLinkList;

//pLinkList m_pList;
//int m_listLength;
int InitList(pLinkList m_pList);
int DestroyList(pLinkList m_pList) ;
int IsEmpty(pLinkList m_pList);
int GetLength(pLinkList m_pList);
int ClearList(pLinkList m_pList);
int SetNodePkt(pLinkList m_pList,int position, AVPacket *newpkt) ;
AVPacket *GetNodePkt(pLinkList m_pList,int position) ;
int InsertNode(pLinkList m_pList,int beforeWhich, AVPacket *pkt) ;
int DeleteNode(pLinkList m_pList,int position);
int GetNode(pLinkList m_pList,int position, LNode **node);


#endif

