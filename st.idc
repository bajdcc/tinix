#define UNLOADED_FILE   1
#include <idc.idc>

static main(void)
{
    Structures();         // structure types
	LowVoids(0x0);
	HighVoids(0xFFEFFFFF);
}

static Structures(void) {
    auto id;
    auto mid;
    BeginTypeUpdating(UTP_STRUCT);
    
    id = AddStrucEx(-1,"file_operations",0);
    id = GetStrucIdByName("file_operations");
    mid = AddStrucMember(id,"seek",	0,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"read",	0x4,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"write",	0x8,	0x20000400,	-1,	4);
    
    
    id = AddStrucEx(-1,"file_req",0);
    id = GetStrucIdByName("file_req");
    mid = AddStrucMember(id,"f_req_state",	0,	0x0000400,	-1,	1);
    mid = AddStrucMember(id,"f_req_orgsize",	0x1,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_req_completed",	0x5,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_req_task",	0x9,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_req_buffer",	0xd,	0x000400,	-1,	512);
    
    id = AddStrucEx(-1,"file",0);
    id = GetStrucIdByName("file");
    mid = AddStrucMember(id,"f_type",	0,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_father",	0x4,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_time",	0x8,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_base",	0xC,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_size",	0x10,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"f_pos",	0x14,	0x20000400,	-1,	256);
    mid = AddStrucMember(id,"f_op",	0x114,	0x20a00400,	GetStrucIdByName("file_operations"),	4);
    mid = AddStrucMember(id,"f_req",	0x118,	0x20a00400,	GetStrucIdByName("file_req"),	4);
    mid = AddStrucMember(id,"f_name",	0x11C,	0x000400,	-1,	16);
    
    
    id = AddStrucEx(-1,"task_struct",0);
    id = GetStrucIdByName("task_struct");
    mid = AddStrucMember(id,"state",	0,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"flags",	0x4,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"father",	0x8,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"pwait",	0xC,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"counter",	0x10,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"user_time",	0x14,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"system_time",	0x18,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"start_time",	0x1C,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"fid",	0x20,	0x20000400,	-1,	0x40);
    mid = AddStrucMember(id,"t_regs",	0x60,	0x20000400,	-1,	0x34);
    mid = AddStrucMember(id,"t_fpu_regs",	0x94,	0x20000400,	-1,	0x6c);
    mid = AddStrucMember(id,"k_stack",	0x100,	0x20000400,	-1,	4);
    mid = AddStrucMember(id,"pgd",	0x104,	0x20000400,	-1,	4);
    
    EndTypeUpdating(UTP_STRUCT);
}

