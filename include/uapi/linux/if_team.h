/*
 * include/linux/if_team.h - Network team device driver header
 * Copyright (c) 2011 Jiri Pirko <jpirko@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _UAPI_LINUX_IF_TEAM_H_
#define _UAPI_LINUX_IF_TEAM_H_


#define TEAM_STRING_MAX_LEN 32

/*                                 
                                  
                                  */

enum {
	TEAM_CMD_NOOP,
	TEAM_CMD_OPTIONS_SET,
	TEAM_CMD_OPTIONS_GET,
	TEAM_CMD_PORT_LIST_GET,

	__TEAM_CMD_MAX,
	TEAM_CMD_MAX = (__TEAM_CMD_MAX - 1),
};

enum {
	TEAM_ATTR_UNSPEC,
	TEAM_ATTR_TEAM_IFINDEX,		/*     */
	TEAM_ATTR_LIST_OPTION,		/*      */
	TEAM_ATTR_LIST_PORT,		/*      */

	__TEAM_ATTR_MAX,
	TEAM_ATTR_MAX = __TEAM_ATTR_MAX - 1,
};

/*                              
  
                          
                           
                              
                           
                              
       
                        
                         
                            
                         
                            
       
 */

enum {
	TEAM_ATTR_ITEM_OPTION_UNSPEC,
	TEAM_ATTR_ITEM_OPTION,		/*      */

	__TEAM_ATTR_ITEM_OPTION_MAX,
	TEAM_ATTR_ITEM_OPTION_MAX = __TEAM_ATTR_ITEM_OPTION_MAX - 1,
};

enum {
	TEAM_ATTR_OPTION_UNSPEC,
	TEAM_ATTR_OPTION_NAME,		/*        */
	TEAM_ATTR_OPTION_CHANGED,	/*      */
	TEAM_ATTR_OPTION_TYPE,		/*    */
	TEAM_ATTR_OPTION_DATA,		/*         */
	TEAM_ATTR_OPTION_REMOVED,	/*      */
	TEAM_ATTR_OPTION_PORT_IFINDEX,	/*     */ /*                      */
	TEAM_ATTR_OPTION_ARRAY_INDEX,	/*     */ /*                   */

	__TEAM_ATTR_OPTION_MAX,
	TEAM_ATTR_OPTION_MAX = __TEAM_ATTR_OPTION_MAX - 1,
};

enum {
	TEAM_ATTR_ITEM_PORT_UNSPEC,
	TEAM_ATTR_ITEM_PORT,		/*      */

	__TEAM_ATTR_ITEM_PORT_MAX,
	TEAM_ATTR_ITEM_PORT_MAX = __TEAM_ATTR_ITEM_PORT_MAX - 1,
};

enum {
	TEAM_ATTR_PORT_UNSPEC,
	TEAM_ATTR_PORT_IFINDEX,		/*     */
	TEAM_ATTR_PORT_CHANGED,		/*      */
	TEAM_ATTR_PORT_LINKUP,		/*      */
	TEAM_ATTR_PORT_SPEED,		/*     */
	TEAM_ATTR_PORT_DUPLEX,		/*    */
	TEAM_ATTR_PORT_REMOVED,		/*      */

	__TEAM_ATTR_PORT_MAX,
	TEAM_ATTR_PORT_MAX = __TEAM_ATTR_PORT_MAX - 1,
};

/*
                               
 */
#define TEAM_GENL_NAME "team"
#define TEAM_GENL_VERSION 0x1
#define TEAM_GENL_CHANGE_EVENT_MC_GRP_NAME "change_event"

#endif /*                        */
