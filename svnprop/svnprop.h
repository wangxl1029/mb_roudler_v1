#pragma once
#define SVNPROP_DATE "$Date: 2017-07-19 08:50:21 +0800 (周三, 2017-07-19) $"
#define SVNPROP_REVISION "$Revision: 5766 $"
#define SVNPROP_AUTHOR "$Author: wangxl $"
#define SVNPROP_HEADURL "$HeadURL: https://10.30.20.5/svn/File/05%E4%B8%AA%E4%BA%BA/040%E7%8E%8B%E6%99%93%E9%BE%99/proj_09_RouteImprove/code/tags/roudler_v1.3.1/svnprop/svnprop.h $"
#define SVNPROP_ID "$Id: svnprop.h 5766 2017-07-19 00:50:21Z wangxl $"



/* History
 * ---------------------------------------------------------
 * $Id: svnprop.h 5766 2017-07-19 00:50:21Z wangxl $
 * 1) print the information of the conflict node
 * ---------------------------------------------------------
 * Revision 5761 2017-07-18 05:24:46Z wangxl
 * 1) implemet quire parent of the specified directed segment ID
 * 2) move send info fuction from roudler doc to roudler doc 
 *	hidden section.
 * 3) add new file revtable.cpp
 * ---------------------------------------------------------
 * Revision 5757 2017-07-17 09:22:32Z wangxl
 * 1) Isolate reverse table to single file
 * 2) Print OK from info by Directied Segment ID 
 * ---------------------------------------------------------
 * Revison 5745 2017-07-14 07:21:16Z wangxl
 * 1) Isolate class reference base to a single file.
 * 2) Change acquirement to a refobj derived class.
 * ---------------------------------------------------------
 * Revision 5739 2017-07-13 08:36:36Z wangxl
 * 1) Implement nsWorkThread::CRevTable initially.
 * 2) Accomplish nsWorkThread::CReferenceBase and it works well
 * ---------------------------------------------------------
 * revision 5739 2017-07-13 08:36:36Z wangxl
 * 1) add nsWorkThread::CReferenceBase to abstract common
 *	reference behavior.
 * ---------------------------------------------------------
 * revison 5739 2017-07-13 08:36:36Z wangxl
 * 1) print empty branch info by command.
 * 2) add data structure to save the branch info for print
 * revison 5737 2017-07-13 06:27:02Z wangxl
 * - new feature :
 * 1) add command parser to Roudler DOC
 * 2) Roudler can know "print json info" and print branch number
 *	and route number to info VIEW.
 * ---------------------------------------------------------
 * revison 5735 2017-07-13 02:57:46Z wangxl
 *	1) add 256 max number of all the mesh
 * ---------------------------------------------------------
 * revison 5722 2017-07-12 03:04:53Z wangxl
 *	1) add version dialog
 *	2) correct ASSERT caused by NULL info hwnd
 */

