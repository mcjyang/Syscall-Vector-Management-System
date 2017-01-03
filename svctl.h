#define SV_CTL_MAGIC 'm'

#define LIST _IOR(SV_CTL_MAGIC, 0, int)
#define CURRENT _IOR(SV_CTL_MAGIC, 1, int)
#define SET _IOW(SV_CTL_MAGIC, 2, int)

#define NAME_LEN_LIMIT 30
#define SV_NUM_LIMIT 10

struct ioctl_record{
	int sv_id;
	int sv_rc;
	char sv_name[NAME_LEN_LIMIT];
};

struct ioctl_arg{
	int p_id;
	int sv_id;
	int sv_total;
	struct ioctl_record records[SV_NUM_LIMIT];
	char sv_name[NAME_LEN_LIMIT];
};