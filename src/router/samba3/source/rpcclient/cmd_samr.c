/*
   Unix SMB/CIFS implementation.
   RPC pipe client

   Copyright (C) Andrew Tridgell              1992-2000,
   Copyright (C) Luke Kenneth Casson Leighton 1996-2000,
   Copyright (C) Elrond                            2000,
   Copyright (C) Tim Potter                        2000
   Copyright (C) Guenther Deschner		   2008

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "includes.h"
#include "rpcclient.h"

extern DOM_SID domain_sid;

/****************************************************************************
 display samr_user_info_7 structure
 ****************************************************************************/
static void display_samr_user_info_7(struct samr_UserInfo7 *r)
{
	printf("\tUser Name   :\t%s\n", r->account_name.string);
}

/****************************************************************************
 display samr_user_info_9 structure
 ****************************************************************************/
static void display_samr_user_info_9(struct samr_UserInfo9 *r)
{
	printf("\tPrimary group RID   :\tox%x\n", r->primary_gid);
}

/****************************************************************************
 display samr_user_info_16 structure
 ****************************************************************************/
static void display_samr_user_info_16(struct samr_UserInfo16 *r)
{
	printf("\tAcct Flags   :\tox%x\n", r->acct_flags);
}

/****************************************************************************
 display samr_user_info_20 structure
 ****************************************************************************/
static void display_samr_user_info_20(struct samr_UserInfo20 *r)
{
	printf("\tRemote Dial :\n");
	dump_data(0, (uint8_t *)r->parameters.array, r->parameters.length*2);
}


/****************************************************************************
 display samr_user_info_21 structure
 ****************************************************************************/
static void display_samr_user_info_21(struct samr_UserInfo21 *r)
{
	printf("\tUser Name   :\t%s\n", r->account_name.string);
	printf("\tFull Name   :\t%s\n", r->full_name.string);
	printf("\tHome Drive  :\t%s\n", r->home_directory.string);
	printf("\tDir Drive   :\t%s\n", r->home_drive.string);
	printf("\tProfile Path:\t%s\n", r->profile_path.string);
	printf("\tLogon Script:\t%s\n", r->logon_script.string);
	printf("\tDescription :\t%s\n", r->description.string);
	printf("\tWorkstations:\t%s\n", r->workstations.string);
	printf("\tComment     :\t%s\n", r->comment.string);
	printf("\tRemote Dial :\n");
	dump_data(0, (uint8_t *)r->parameters.array, r->parameters.length*2);

	printf("\tLogon Time               :\t%s\n",
	       http_timestring(nt_time_to_unix(r->last_logon)));
	printf("\tLogoff Time              :\t%s\n",
	       http_timestring(nt_time_to_unix(r->last_logoff)));
	printf("\tKickoff Time             :\t%s\n",
	       http_timestring(nt_time_to_unix(r->acct_expiry)));
	printf("\tPassword last set Time   :\t%s\n",
	       http_timestring(nt_time_to_unix(r->last_password_change)));
	printf("\tPassword can change Time :\t%s\n",
	       http_timestring(nt_time_to_unix(r->allow_password_change)));
	printf("\tPassword must change Time:\t%s\n",
	       http_timestring(nt_time_to_unix(r->force_password_change)));

	printf("\tunknown_2[0..31]...\n"); /* user passwords? */

	printf("\tuser_rid :\t0x%x\n"  , r->rid); /* User ID */
	printf("\tgroup_rid:\t0x%x\n"  , r->primary_gid); /* Group ID */
	printf("\tacb_info :\t0x%08x\n", r->acct_flags); /* Account Control Info */

	printf("\tfields_present:\t0x%08x\n", r->fields_present); /* 0x00ff ffff */
	printf("\tlogon_divs:\t%d\n", r->logon_hours.units_per_week); /* 0x0000 00a8 which is 168 which is num hrs in a week */
	printf("\tbad_password_count:\t0x%08x\n", r->bad_password_count);
	printf("\tlogon_count:\t0x%08x\n", r->logon_count);

	printf("\tpadding1[0..7]...\n");

	if (r->logon_hours.bits) {
		printf("\tlogon_hrs[0..%d]...\n", r->logon_hours.units_per_week/8);
	}
}


static void display_password_properties(uint32_t password_properties)
{
	printf("password_properties: 0x%08x\n", password_properties);

	if (password_properties & DOMAIN_PASSWORD_COMPLEX)
		printf("\tDOMAIN_PASSWORD_COMPLEX\n");

	if (password_properties & DOMAIN_PASSWORD_NO_ANON_CHANGE)
		printf("\tDOMAIN_PASSWORD_NO_ANON_CHANGE\n");

	if (password_properties & DOMAIN_PASSWORD_NO_CLEAR_CHANGE)
		printf("\tDOMAIN_PASSWORD_NO_CLEAR_CHANGE\n");

	if (password_properties & DOMAIN_PASSWORD_LOCKOUT_ADMINS)
		printf("\tDOMAIN_PASSWORD_LOCKOUT_ADMINS\n");

	if (password_properties & DOMAIN_PASSWORD_STORE_CLEARTEXT)
		printf("\tDOMAIN_PASSWORD_STORE_CLEARTEXT\n");

	if (password_properties & DOMAIN_REFUSE_PASSWORD_CHANGE)
		printf("\tDOMAIN_REFUSE_PASSWORD_CHANGE\n");
}

static void display_sam_dom_info_1(struct samr_DomInfo1 *info1)
{
	printf("Minimum password length:\t\t\t%d\n",
		info1->min_password_length);
	printf("Password uniqueness (remember x passwords):\t%d\n",
		info1->password_history_length);
	display_password_properties(info1->password_properties);
	printf("password expire in:\t\t\t\t%s\n",
		display_time(info1->max_password_age));
	printf("Min password age (allow changing in x days):\t%s\n",
		display_time(info1->min_password_age));
}

static void display_sam_dom_info_2(struct samr_DomInfo2 *info2)
{
	printf("Domain:\t\t%s\n", info2->domain_name.string);
	printf("Server:\t\t%s\n", info2->primary.string);
	printf("Comment:\t%s\n", info2->comment.string);

	printf("Total Users:\t%d\n", info2->num_users);
	printf("Total Groups:\t%d\n", info2->num_groups);
	printf("Total Aliases:\t%d\n", info2->num_aliases);

	printf("Sequence No:\t%llu\n", (unsigned long long)info2->sequence_num);

	printf("Force Logoff:\t%d\n",
		(int)nt_time_to_unix_abs(&info2->force_logoff_time));

	printf("Unknown 2:\t0x%x\n", info2->unknown2);
	printf("Server Role:\t%s\n", server_role_str(info2->role));
	printf("Unknown 3:\t0x%x\n", info2->unknown3);
}

static void display_sam_dom_info_3(struct samr_DomInfo3 *info3)
{
	printf("Force Logoff:\t%d\n",
		(int)nt_time_to_unix_abs(&info3->force_logoff_time));
}

static void display_sam_dom_info_4(struct samr_DomInfo4 *info4)
{
	printf("Comment:\t%s\n", info4->comment.string);
}

static void display_sam_dom_info_5(struct samr_DomInfo5 *info5)
{
	printf("Domain:\t\t%s\n", info5->domain_name.string);
}

static void display_sam_dom_info_6(struct samr_DomInfo6 *info6)
{
	printf("Server:\t\t%s\n", info6->primary.string);
}

static void display_sam_dom_info_7(struct samr_DomInfo7 *info7)
{
	printf("Server Role:\t%s\n", server_role_str(info7->role));
}

static void display_sam_dom_info_8(struct samr_DomInfo8 *info8)
{
	printf("Sequence No:\t%llu\n", (unsigned long long)info8->sequence_num);
	printf("Domain Create Time:\t%s\n",
		http_timestring(nt_time_to_unix(info8->domain_create_time)));
}

static void display_sam_dom_info_9(struct samr_DomInfo9 *info9)
{
	printf("unknown:\t%d (0x%08x)\n", info9->unknown, info9->unknown);
}

static void display_sam_dom_info_12(struct samr_DomInfo12 *info12)
{
	printf("Bad password lockout duration:               %s\n",
		display_time(info12->lockout_duration));
	printf("Reset Lockout after:                         %s\n",
		display_time(info12->lockout_window));
	printf("Lockout after bad attempts:                  %d\n",
		info12->lockout_threshold);
}

static void display_sam_dom_info_13(struct samr_DomInfo13 *info13)
{
	printf("Sequence No:\t%llu\n", (unsigned long long)info13->sequence_num);
	printf("Domain Create Time:\t%s\n",
		http_timestring(nt_time_to_unix(info13->domain_create_time)));
	printf("Sequence No at last promotion:\t%llu\n",
		(unsigned long long)info13->modified_count_at_last_promotion);
}

static void display_sam_info_1(struct samr_DispEntryGeneral *r)
{
	printf("index: 0x%x ", r->idx);
	printf("RID: 0x%x ", r->rid);
	printf("acb: 0x%08x ", r->acct_flags);
	printf("Account: %s\t", r->account_name.string);
	printf("Name: %s\t", r->full_name.string);
	printf("Desc: %s\n", r->description.string);
}

static void display_sam_info_2(struct samr_DispEntryFull *r)
{
	printf("index: 0x%x ", r->idx);
	printf("RID: 0x%x ", r->rid);
	printf("acb: 0x%08x ", r->acct_flags);
	printf("Account: %s\t", r->account_name.string);
	printf("Desc: %s\n", r->description.string);
}

static void display_sam_info_3(struct samr_DispEntryFullGroup *r)
{
	printf("index: 0x%x ", r->idx);
	printf("RID: 0x%x ", r->rid);
	printf("acb: 0x%08x ", r->acct_flags);
	printf("Account: %s\t", r->account_name.string);
	printf("Desc: %s\n", r->description.string);
}

static void display_sam_info_4(struct samr_DispEntryAscii *r)
{
	printf("index: 0x%x ", r->idx);
	printf("Account: %s\n", r->account_name.string);
}

static void display_sam_info_5(struct samr_DispEntryAscii *r)
{
	printf("index: 0x%x ", r->idx);
	printf("Account: %s\n", r->account_name.string);
}

/**********************************************************************
 * Query user information
 */
static NTSTATUS cmd_samr_query_user(struct rpc_pipe_client *cli,
                                    TALLOC_CTX *mem_ctx,
                                    int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, user_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 info_level = 21;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	union samr_UserInfo *info = NULL;
	uint32 user_rid = 0;

	if ((argc < 2) || (argc > 4)) {
		printf("Usage: %s rid [info level] [access mask] \n", argv[0]);
		return NT_STATUS_OK;
	}

	sscanf(argv[1], "%i", &user_rid);

	if (argc > 2)
		sscanf(argv[2], "%i", &info_level);

	if (argc > 3)
		sscanf(argv[3], "%x", &access_mask);


	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					MAXIMUM_ALLOWED_ACCESS,
					&domain_sid,
					&domain_pol);
	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenUser(cli, mem_ctx,
				      &domain_pol,
				      access_mask,
				      user_rid,
				      &user_pol);

	if (NT_STATUS_EQUAL(result, NT_STATUS_NO_SUCH_USER) &&
	    (user_rid == 0)) {

		/* Probably this was a user name, try lookupnames */
		struct samr_Ids rids, types;
		struct lsa_String lsa_acct_name;

		init_lsa_String(&lsa_acct_name, argv[1]);

		result = rpccli_samr_LookupNames(cli, mem_ctx,
						 &domain_pol,
						 1,
						 &lsa_acct_name,
						 &rids,
						 &types);

		if (NT_STATUS_IS_OK(result)) {
			result = rpccli_samr_OpenUser(cli, mem_ctx,
						      &domain_pol,
						      access_mask,
						      rids.ids[0],
						      &user_pol);
		}
	}


	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_QueryUserInfo(cli, mem_ctx,
					   &user_pol,
					   info_level,
					   &info);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	switch (info_level) {
	case 7:
		display_samr_user_info_7(&info->info7);
		break;
	case 9:
		display_samr_user_info_9(&info->info9);
		break;
	case 16:
		display_samr_user_info_16(&info->info16);
		break;
	case 20:
		display_samr_user_info_20(&info->info20);
		break;
	case 21:
		display_samr_user_info_21(&info->info21);
		break;
	default:
		printf("Unsupported infolevel: %d\n", info_level);
		break;
	}

	rpccli_samr_Close(cli, mem_ctx, &user_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);

done:
	return result;
}

/****************************************************************************
 display group info
 ****************************************************************************/
static void display_group_info1(struct samr_GroupInfoAll *info1)
{
	printf("\tGroup Name:\t%s\n", info1->name.string);
	printf("\tDescription:\t%s\n", info1->description.string);
	printf("\tGroup Attribute:%d\n", info1->attributes);
	printf("\tNum Members:%d\n", info1->num_members);
}

/****************************************************************************
 display group info
 ****************************************************************************/
static void display_group_info2(struct lsa_String *info2)
{
	printf("\tGroup Description:%s\n", info2->string);
}


/****************************************************************************
 display group info
 ****************************************************************************/
static void display_group_info3(struct samr_GroupInfoAttributes *info3)
{
	printf("\tGroup Attribute:%d\n", info3->attributes);
}


/****************************************************************************
 display group info
 ****************************************************************************/
static void display_group_info4(struct lsa_String *info4)
{
	printf("\tGroup Description:%s\n", info4->string);
}

/****************************************************************************
 display group info
 ****************************************************************************/
static void display_group_info5(struct samr_GroupInfoAll *info5)
{
	printf("\tGroup Name:\t%s\n", info5->name.string);
	printf("\tDescription:\t%s\n", info5->description.string);
	printf("\tGroup Attribute:%d\n", info5->attributes);
	printf("\tNum Members:%d\n", info5->num_members);
}

/****************************************************************************
 display sam sync structure
 ****************************************************************************/
static void display_group_info(union samr_GroupInfo *info,
			       enum samr_GroupInfoEnum level)
{
	switch (level) {
		case 1:
			display_group_info1(&info->all);
			break;
		case 2:
			display_group_info2(&info->name);
			break;
		case 3:
			display_group_info3(&info->attributes);
			break;
		case 4:
			display_group_info4(&info->description);
			break;
		case 5:
			display_group_info5(&info->all2);
			break;
	}
}

/***********************************************************************
 * Query group information
 */
static NTSTATUS cmd_samr_query_group(struct rpc_pipe_client *cli,
                                     TALLOC_CTX *mem_ctx,
                                     int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, group_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	enum samr_GroupInfoEnum info_level = GROUPINFOALL;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	union samr_GroupInfo *group_info = NULL;
	uint32 group_rid;

	if ((argc < 2) || (argc > 4)) {
		printf("Usage: %s rid [info level] [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

        sscanf(argv[1], "%i", &group_rid);

	if (argc > 2)
		info_level = atoi(argv[2]);

	if (argc > 3)
		sscanf(argv[3], "%x", &access_mask);

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					MAXIMUM_ALLOWED_ACCESS,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenGroup(cli, mem_ctx,
				       &domain_pol,
				       access_mask,
				       group_rid,
				       &group_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_QueryGroupInfo(cli, mem_ctx,
					    &group_pol,
					    info_level,
					    &group_info);
	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	display_group_info(group_info, info_level);

	rpccli_samr_Close(cli, mem_ctx, &group_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
done:
	return result;
}

/* Query groups a user is a member of */

static NTSTATUS cmd_samr_query_usergroups(struct rpc_pipe_client *cli,
                                          TALLOC_CTX *mem_ctx,
                                          int argc, const char **argv)
{
	POLICY_HND 		connect_pol,
				domain_pol,
				user_pol;
	NTSTATUS		result = NT_STATUS_UNSUCCESSFUL;
	uint32 			user_rid;
	uint32			access_mask = MAXIMUM_ALLOWED_ACCESS;
	int 			i;
	struct samr_RidWithAttributeArray *rid_array = NULL;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s rid [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	sscanf(argv[1], "%i", &user_rid);

	if (argc > 2)
		sscanf(argv[2], "%x", &access_mask);

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					MAXIMUM_ALLOWED_ACCESS,
					&domain_sid, &domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenUser(cli, mem_ctx,
				      &domain_pol,
				      access_mask,
				      user_rid,
				      &user_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_GetGroupsForUser(cli, mem_ctx,
					      &user_pol,
					      &rid_array);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	for (i = 0; i < rid_array->count; i++) {
		printf("\tgroup rid:[0x%x] attr:[0x%x]\n",
		       rid_array->rids[i].rid,
		       rid_array->rids[i].attributes);
	}

	rpccli_samr_Close(cli, mem_ctx, &user_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

/* Query aliases a user is a member of */

static NTSTATUS cmd_samr_query_useraliases(struct rpc_pipe_client *cli,
					   TALLOC_CTX *mem_ctx,
					   int argc, const char **argv)
{
	POLICY_HND 		connect_pol, domain_pol;
	NTSTATUS		result = NT_STATUS_UNSUCCESSFUL;
	DOM_SID                *sids;
	size_t                     num_sids;
	uint32			access_mask = MAXIMUM_ALLOWED_ACCESS;
	int 			i;
	struct lsa_SidArray sid_array;
	struct samr_Ids alias_rids;

	if (argc < 3) {
		printf("Usage: %s builtin|domain sid1 sid2 ...\n", argv[0]);
		return NT_STATUS_INVALID_PARAMETER;
	}

	sids = NULL;
	num_sids = 0;

	for (i=2; i<argc; i++) {
		DOM_SID tmp_sid;
		if (!string_to_sid(&tmp_sid, argv[i])) {
			printf("%s is not a legal SID\n", argv[i]);
			return NT_STATUS_INVALID_PARAMETER;
		}
		result = add_sid_to_array(mem_ctx, &tmp_sid, &sids, &num_sids);
		if (!NT_STATUS_IS_OK(result)) {
			return result;
		}
	}

	if (num_sids) {
		sid_array.sids = TALLOC_ZERO_ARRAY(mem_ctx, struct lsa_SidPtr, num_sids);
		if (sid_array.sids == NULL)
			return NT_STATUS_NO_MEMORY;
	} else {
		sid_array.sids = NULL;
	}

	for (i=0; i<num_sids; i++) {
		sid_array.sids[i].sid = sid_dup_talloc(mem_ctx, &sids[i]);
		if (!sid_array.sids[i].sid) {
			return NT_STATUS_NO_MEMORY;
		}
	}

	sid_array.num_sids = num_sids;

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	if (StrCaseCmp(argv[1], "domain")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						access_mask,
						&domain_sid, &domain_pol);
	else if (StrCaseCmp(argv[1], "builtin")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						access_mask,
						CONST_DISCARD(struct dom_sid2 *, &global_sid_Builtin),
						&domain_pol);
	else {
		printf("Usage: %s builtin|domain sid1 sid2 ...\n", argv[0]);
		return NT_STATUS_INVALID_PARAMETER;
	}

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_GetAliasMembership(cli, mem_ctx,
						&domain_pol,
						&sid_array,
						&alias_rids);
	if (!NT_STATUS_IS_OK(result))
		goto done;

	for (i = 0; i < alias_rids.count; i++) {
		printf("\tgroup rid:[0x%x]\n", alias_rids.ids[i]);
	}

	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

/* Query members of a group */

static NTSTATUS cmd_samr_query_groupmem(struct rpc_pipe_client *cli,
                                        TALLOC_CTX *mem_ctx,
                                        int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, group_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 group_rid;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	int i;
	unsigned int old_timeout;
	struct samr_RidTypeArray *rids = NULL;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s rid [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	sscanf(argv[1], "%i", &group_rid);

	if (argc > 2)
		sscanf(argv[2], "%x", &access_mask);

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					MAXIMUM_ALLOWED_ACCESS,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenGroup(cli, mem_ctx,
				       &domain_pol,
				       access_mask,
				       group_rid,
				       &group_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Make sure to wait for our DC's reply */
	old_timeout = rpccli_set_timeout(cli, 30000); /* 30 seconds. */
	rpccli_set_timeout(cli, MAX(30000, old_timeout)); /* At least 30 sec */

	result = rpccli_samr_QueryGroupMember(cli, mem_ctx,
					      &group_pol,
					      &rids);

	rpccli_set_timeout(cli, old_timeout);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	for (i = 0; i < rids->count; i++) {
		printf("\trid:[0x%x] attr:[0x%x]\n", rids->rids[i],
		       rids->types[i]);
	}

	rpccli_samr_Close(cli, mem_ctx, &group_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

/* Enumerate domain users */

static NTSTATUS cmd_samr_enum_dom_users(struct rpc_pipe_client *cli,
					TALLOC_CTX *mem_ctx,
					int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 start_idx, size, num_dom_users, i;
	struct samr_SamArray *dom_users = NULL;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	uint32 acb_mask = ACB_NORMAL;
	bool got_connect_pol = False, got_domain_pol = False;

	if ((argc < 1) || (argc > 3)) {
		printf("Usage: %s [access_mask] [acb_mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc > 1)
		sscanf(argv[1], "%x", &access_mask);

	if (argc > 2)
		sscanf(argv[2], "%x", &acb_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	got_connect_pol = True;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	got_domain_pol = True;

	/* Enumerate domain users */

	start_idx = 0;
	size = 0xffff;

	do {
		result = rpccli_samr_EnumDomainUsers(cli, mem_ctx,
						     &domain_pol,
						     &start_idx,
						     acb_mask,
						     &dom_users,
						     size,
						     &num_dom_users);

		if (NT_STATUS_IS_OK(result) ||
		    NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES)) {

			for (i = 0; i < num_dom_users; i++)
                               printf("user:[%s] rid:[0x%x]\n",
				       dom_users->entries[i].name.string,
				       dom_users->entries[i].idx);
		}

	} while (NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES));

 done:
	if (got_domain_pol)
		rpccli_samr_Close(cli, mem_ctx, &domain_pol);

	if (got_connect_pol)
		rpccli_samr_Close(cli, mem_ctx, &connect_pol);

	return result;
}

/* Enumerate domain groups */

static NTSTATUS cmd_samr_enum_dom_groups(struct rpc_pipe_client *cli,
                                         TALLOC_CTX *mem_ctx,
                                         int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 start_idx, size, num_dom_groups, i;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	struct samr_SamArray *dom_groups = NULL;
	bool got_connect_pol = False, got_domain_pol = False;

	if ((argc < 1) || (argc > 2)) {
		printf("Usage: %s [access_mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc > 1)
		sscanf(argv[1], "%x", &access_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	got_connect_pol = True;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	got_domain_pol = True;

	/* Enumerate domain groups */

	start_idx = 0;
	size = 0xffff;

	do {
		result = rpccli_samr_EnumDomainGroups(cli, mem_ctx,
						      &domain_pol,
						      &start_idx,
						      &dom_groups,
						      size,
						      &num_dom_groups);
		if (NT_STATUS_IS_OK(result) ||
		    NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES)) {

			for (i = 0; i < num_dom_groups; i++)
				printf("group:[%s] rid:[0x%x]\n",
				       dom_groups->entries[i].name.string,
				       dom_groups->entries[i].idx);
		}

	} while (NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES));

 done:
	if (got_domain_pol)
		rpccli_samr_Close(cli, mem_ctx, &domain_pol);

	if (got_connect_pol)
		rpccli_samr_Close(cli, mem_ctx, &connect_pol);

	return result;
}

/* Enumerate alias groups */

static NTSTATUS cmd_samr_enum_als_groups(struct rpc_pipe_client *cli,
                                         TALLOC_CTX *mem_ctx,
                                         int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 start_idx, size, num_als_groups, i;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	struct samr_SamArray *als_groups = NULL;
	bool got_connect_pol = False, got_domain_pol = False;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s builtin|domain [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc > 2)
		sscanf(argv[2], "%x", &access_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	got_connect_pol = True;

	/* Get domain policy handle */

	if (StrCaseCmp(argv[1], "domain")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						access_mask,
						&domain_sid,
						&domain_pol);
	else if (StrCaseCmp(argv[1], "builtin")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						access_mask,
						CONST_DISCARD(struct dom_sid2 *, &global_sid_Builtin),
						&domain_pol);
	else
		return NT_STATUS_OK;

	if (!NT_STATUS_IS_OK(result))
		goto done;

	got_domain_pol = True;

	/* Enumerate alias groups */

	start_idx = 0;
	size = 0xffff;		/* Number of groups to retrieve */

	do {
		result = rpccli_samr_EnumDomainAliases(cli, mem_ctx,
						       &domain_pol,
						       &start_idx,
						       &als_groups,
						       size,
						       &num_als_groups);

		if (NT_STATUS_IS_OK(result) ||
		    NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES)) {

			for (i = 0; i < num_als_groups; i++)
				printf("group:[%s] rid:[0x%x]\n",
				       als_groups->entries[i].name.string,
				       als_groups->entries[i].idx);
		}
	} while (NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES));

 done:
	if (got_domain_pol)
		rpccli_samr_Close(cli, mem_ctx, &domain_pol);

	if (got_connect_pol)
		rpccli_samr_Close(cli, mem_ctx, &connect_pol);

	return result;
}

/* Enumerate domains */

static NTSTATUS cmd_samr_enum_domains(struct rpc_pipe_client *cli,
				      TALLOC_CTX *mem_ctx,
				      int argc, const char **argv)
{
	POLICY_HND connect_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 start_idx, size, num_entries, i;
	uint32 access_mask = SEC_FLAG_MAXIMUM_ALLOWED;
	bool got_connect_pol = false;
	struct samr_SamArray *sam = NULL;

	if ((argc < 1) || (argc > 2)) {
		printf("Usage: %s [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc > 1) {
		sscanf(argv[1], "%x", &access_mask);
	}

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  access_mask,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	got_connect_pol = true;

	/* Enumerate alias groups */

	start_idx = 0;
	size = 0xffff;

	do {
		result = rpccli_samr_EnumDomains(cli, mem_ctx,
						 &connect_pol,
						 &start_idx,
						 &sam,
						 size,
						 &num_entries);

		if (NT_STATUS_IS_OK(result) ||
		    NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES)) {

			for (i = 0; i < num_entries; i++)
				printf("name:[%s] idx:[0x%x]\n",
				       sam->entries[i].name.string,
				       sam->entries[i].idx);
		}
	} while (NT_STATUS_V(result) == NT_STATUS_V(STATUS_MORE_ENTRIES));

 done:
	if (got_connect_pol) {
		rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	}

	return result;
}


/* Query alias membership */

static NTSTATUS cmd_samr_query_aliasmem(struct rpc_pipe_client *cli,
                                        TALLOC_CTX *mem_ctx,
                                        int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, alias_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 alias_rid, i;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	struct lsa_SidArray sid_array;

	if ((argc < 3) || (argc > 4)) {
		printf("Usage: %s builtin|domain rid [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	sscanf(argv[2], "%i", &alias_rid);

	if (argc > 3)
		sscanf(argv[3], "%x", &access_mask);

	/* Open SAMR handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Open handle on domain */

	if (StrCaseCmp(argv[1], "domain")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						&domain_sid,
						&domain_pol);
	else if (StrCaseCmp(argv[1], "builtin")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						CONST_DISCARD(struct dom_sid2 *, &global_sid_Builtin),
						&domain_pol);
	else
		return NT_STATUS_OK;

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Open handle on alias */

	result = rpccli_samr_OpenAlias(cli, mem_ctx,
				       &domain_pol,
				       access_mask,
				       alias_rid,
				       &alias_pol);
	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_GetMembersInAlias(cli, mem_ctx,
					       &alias_pol,
					       &sid_array);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	for (i = 0; i < sid_array.num_sids; i++) {
		fstring sid_str;

		sid_to_fstring(sid_str, sid_array.sids[i].sid);
		printf("\tsid:[%s]\n", sid_str);
	}

	rpccli_samr_Close(cli, mem_ctx, &alias_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

/* Query alias info */

static NTSTATUS cmd_samr_query_aliasinfo(struct rpc_pipe_client *cli,
					 TALLOC_CTX *mem_ctx,
					 int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, alias_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32_t alias_rid;
	uint32_t access_mask = SEC_FLAG_MAXIMUM_ALLOWED;
	union samr_AliasInfo *info = NULL;
	enum samr_AliasInfoEnum level = ALIASINFOALL;

	if ((argc < 3) || (argc > 4)) {
		printf("Usage: %s builtin|domain rid [level] [access mask]\n",
			argv[0]);
		return NT_STATUS_OK;
	}

	sscanf(argv[2], "%i", &alias_rid);

	if (argc > 2) {
		level = atoi(argv[3]);
	}

	if (argc > 3) {
		sscanf(argv[4], "%x", &access_mask);
	}

	/* Open SAMR handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  SEC_FLAG_MAXIMUM_ALLOWED,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	/* Open handle on domain */

	if (strequal(argv[1], "domain")) {

		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						SEC_FLAG_MAXIMUM_ALLOWED,
						&domain_sid,
						&domain_pol);

	} else if (strequal(argv[1], "builtin")) {

		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						SEC_FLAG_MAXIMUM_ALLOWED,
						CONST_DISCARD(struct dom_sid2 *, &global_sid_Builtin),
						&domain_pol);

	} else {
		return NT_STATUS_OK;
	}

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	/* Open handle on alias */

	result = rpccli_samr_OpenAlias(cli, mem_ctx,
				       &domain_pol,
				       access_mask,
				       alias_rid,
				       &alias_pol);
	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	result = rpccli_samr_QueryAliasInfo(cli, mem_ctx,
					    &alias_pol,
					    level,
					    &info);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	switch (level) {
		case ALIASINFOALL:
			printf("Name: %s\n", info->all.name.string);
			printf("Description: %s\n", info->all.description.string);
			printf("Num Members: %d\n", info->all.num_members);
			break;
		case ALIASINFONAME:
			printf("Name: %s\n", info->name.string);
			break;
		case ALIASINFODESCRIPTION:
			printf("Description: %s\n", info->description.string);
			break;
		default:
			break;
	}

	rpccli_samr_Close(cli, mem_ctx, &alias_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}


/* Query delete an alias membership */

static NTSTATUS cmd_samr_delete_alias(struct rpc_pipe_client *cli,
				      TALLOC_CTX *mem_ctx,
				      int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, alias_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 alias_rid;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;

	if (argc != 3) {
		printf("Usage: %s builtin|domain [rid|name]\n", argv[0]);
		return NT_STATUS_OK;
	}

	alias_rid = strtoul(argv[2], NULL, 10);

	/* Open SAMR handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Open handle on domain */

	if (StrCaseCmp(argv[1], "domain")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						&domain_sid,
						&domain_pol);
	else if (StrCaseCmp(argv[1], "builtin")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						CONST_DISCARD(struct dom_sid2 *, &global_sid_Builtin),
						&domain_pol);
	else
		return NT_STATUS_INVALID_PARAMETER;

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Open handle on alias */

	result = rpccli_samr_OpenAlias(cli, mem_ctx,
				       &domain_pol,
				       access_mask,
				       alias_rid,
				       &alias_pol);
	if (!NT_STATUS_IS_OK(result) && (alias_rid == 0)) {
		/* Probably this was a user name, try lookupnames */
		struct samr_Ids rids, types;
		struct lsa_String lsa_acct_name;

		init_lsa_String(&lsa_acct_name, argv[2]);

		result = rpccli_samr_LookupNames(cli, mem_ctx,
						 &domain_pol,
						 1,
						 &lsa_acct_name,
						 &rids,
						 &types);

		if (NT_STATUS_IS_OK(result)) {
			result = rpccli_samr_OpenAlias(cli, mem_ctx,
						       &domain_pol,
						       access_mask,
						       rids.ids[0],
						       &alias_pol);
		}
	}

	result = rpccli_samr_DeleteDomAlias(cli, mem_ctx,
					    &alias_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

/* Query display info */

static NTSTATUS cmd_samr_query_dispinfo_internal(struct rpc_pipe_client *cli,
						 TALLOC_CTX *mem_ctx,
						 int argc, const char **argv,
						 uint32_t opcode)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 start_idx=0, max_entries=250, max_size = 0xffff, num_entries = 0, i;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	uint32 info_level = 1;
	union samr_DispInfo info;
	int loop_count = 0;
	bool got_params = False; /* Use get_query_dispinfo_params() or not? */
	uint32_t total_size, returned_size;

	if (argc > 6) {
		printf("Usage: %s [info level] [start index] [max entries] [max size] [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc >= 2)
                sscanf(argv[1], "%i", &info_level);

	if (argc >= 3)
                sscanf(argv[2], "%i", &start_idx);

	if (argc >= 4) {
                sscanf(argv[3], "%i", &max_entries);
		got_params = True;
	}

	if (argc >= 5) {
                sscanf(argv[4], "%i", &max_size);
		got_params = True;
	}

	if (argc >= 6)
                sscanf(argv[5], "%x", &access_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Query display info */

	do {

		if (!got_params)
			get_query_dispinfo_params(
				loop_count, &max_entries, &max_size);

		switch (opcode) {
		case NDR_SAMR_QUERYDISPLAYINFO:
			result = rpccli_samr_QueryDisplayInfo(cli, mem_ctx,
							      &domain_pol,
							      info_level,
							      start_idx,
							      max_entries,
							      max_size,
							      &total_size,
							      &returned_size,
							      &info);
			break;
		case NDR_SAMR_QUERYDISPLAYINFO2:
			result = rpccli_samr_QueryDisplayInfo2(cli, mem_ctx,
							       &domain_pol,
							       info_level,
							       start_idx,
							       max_entries,
							       max_size,
							       &total_size,
							       &returned_size,
							       &info);

			break;
		case NDR_SAMR_QUERYDISPLAYINFO3:
			result = rpccli_samr_QueryDisplayInfo3(cli, mem_ctx,
							       &domain_pol,
							       info_level,
							       start_idx,
							       max_entries,
							       max_size,
							       &total_size,
							       &returned_size,
							       &info);

			break;
		default:
			return NT_STATUS_INVALID_PARAMETER;
		}

		if (!NT_STATUS_IS_OK(result) &&
		    !NT_STATUS_EQUAL(result, NT_STATUS_NO_MORE_ENTRIES) &&
		    !NT_STATUS_EQUAL(result, STATUS_MORE_ENTRIES)) {
			break;
		}

		loop_count++;

		switch (info_level) {
			case 1:
				num_entries = info.info1.count;
				break;
			case 2:
				num_entries = info.info2.count;
				break;
			case 3:
				num_entries = info.info3.count;
				break;
			case 4:
				num_entries = info.info4.count;
				break;
			case 5:
				num_entries = info.info5.count;
				break;
			default:
				break;
		}

		start_idx += num_entries;

		if (num_entries == 0)
			break;

		for (i = 0; i < num_entries; i++) {
			switch (info_level) {
			case 1:
				display_sam_info_1(&info.info1.entries[i]);
				break;
			case 2:
				display_sam_info_2(&info.info2.entries[i]);
				break;
			case 3:
				display_sam_info_3(&info.info3.entries[i]);
				break;
			case 4:
				display_sam_info_4(&info.info4.entries[i]);
				break;
			case 5:
				display_sam_info_5(&info.info5.entries[i]);
				break;
			}
		}
	} while ( NT_STATUS_EQUAL(result, STATUS_MORE_ENTRIES));

	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

static NTSTATUS cmd_samr_query_dispinfo(struct rpc_pipe_client *cli,
					TALLOC_CTX *mem_ctx,
					int argc, const char **argv)
{
	return cmd_samr_query_dispinfo_internal(cli, mem_ctx, argc, argv,
						NDR_SAMR_QUERYDISPLAYINFO);
}

static NTSTATUS cmd_samr_query_dispinfo2(struct rpc_pipe_client *cli,
					 TALLOC_CTX *mem_ctx,
					 int argc, const char **argv)
{
	return cmd_samr_query_dispinfo_internal(cli, mem_ctx, argc, argv,
						NDR_SAMR_QUERYDISPLAYINFO2);
}

static NTSTATUS cmd_samr_query_dispinfo3(struct rpc_pipe_client *cli,
					 TALLOC_CTX *mem_ctx,
					 int argc, const char **argv)
{
	return cmd_samr_query_dispinfo_internal(cli, mem_ctx, argc, argv,
						NDR_SAMR_QUERYDISPLAYINFO3);
}

/* Query domain info */

static NTSTATUS cmd_samr_query_dominfo(struct rpc_pipe_client *cli,
                                       TALLOC_CTX *mem_ctx,
                                       int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 switch_level = 2;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	union samr_DomainInfo *info = NULL;

	if (argc > 3) {
		printf("Usage: %s [info level] [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc > 1)
                sscanf(argv[1], "%i", &switch_level);

	if (argc > 2)
                sscanf(argv[2], "%x", &access_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Query domain info */

	result = rpccli_samr_QueryDomainInfo(cli, mem_ctx,
					     &domain_pol,
					     switch_level,
					     &info);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Display domain info */

	switch (switch_level) {
	case 1:
		display_sam_dom_info_1(&info->info1);
		break;
	case 2:
		display_sam_dom_info_2(&info->info2);
		break;
	case 3:
		display_sam_dom_info_3(&info->info3);
		break;
	case 4:
		display_sam_dom_info_4(&info->info4);
		break;
	case 5:
		display_sam_dom_info_5(&info->info5);
		break;
	case 6:
		display_sam_dom_info_6(&info->info6);
		break;
	case 7:
		display_sam_dom_info_7(&info->info7);
		break;
	case 8:
		display_sam_dom_info_8(&info->info8);
		break;
	case 9:
		display_sam_dom_info_9(&info->info9);
		break;
	case 12:
		display_sam_dom_info_12(&info->info12);
		break;
	case 13:
		display_sam_dom_info_13(&info->info13);
		break;

	default:
		printf("cannot display domain info for switch value %d\n",
		       switch_level);
		break;
	}

 done:

	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	return result;
}

/* Create domain user */

static NTSTATUS cmd_samr_create_dom_user(struct rpc_pipe_client *cli,
                                         TALLOC_CTX *mem_ctx,
                                         int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, user_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	struct lsa_String acct_name;
	uint32 acb_info;
	uint32 acct_flags, user_rid;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	uint32_t access_granted = 0;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s username [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	init_lsa_String(&acct_name, argv[1]);

	if (argc > 2)
                sscanf(argv[2], "%x", &access_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Create domain user */

	acb_info = ACB_NORMAL;
	acct_flags = SEC_GENERIC_READ | SEC_GENERIC_WRITE | SEC_GENERIC_EXECUTE |
		     SEC_STD_WRITE_DAC | SEC_STD_DELETE |
		     SAMR_USER_ACCESS_SET_PASSWORD |
		     SAMR_USER_ACCESS_GET_ATTRIBUTES |
		     SAMR_USER_ACCESS_SET_ATTRIBUTES;

	result = rpccli_samr_CreateUser2(cli, mem_ctx,
					 &domain_pol,
					 &acct_name,
					 acb_info,
					 acct_flags,
					 &user_pol,
					 &access_granted,
					 &user_rid);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &user_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

 done:
	return result;
}

/* Create domain group */

static NTSTATUS cmd_samr_create_dom_group(struct rpc_pipe_client *cli,
                                          TALLOC_CTX *mem_ctx,
                                          int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, group_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	struct lsa_String grp_name;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	uint32_t rid = 0;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s groupname [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	init_lsa_String(&grp_name, argv[1]);

	if (argc > 2)
                sscanf(argv[2], "%x", &access_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Create domain user */
	result = rpccli_samr_CreateDomainGroup(cli, mem_ctx,
					       &domain_pol,
					       &grp_name,
					       MAXIMUM_ALLOWED_ACCESS,
					       &group_pol,
					       &rid);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &group_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

 done:
	return result;
}

/* Create domain alias */

static NTSTATUS cmd_samr_create_dom_alias(struct rpc_pipe_client *cli,
                                          TALLOC_CTX *mem_ctx,
                                          int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, alias_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	struct lsa_String alias_name;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	uint32_t rid = 0;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s aliasname [access mask]\n", argv[0]);
		return NT_STATUS_OK;
	}

	init_lsa_String(&alias_name, argv[1]);

	if (argc > 2)
                sscanf(argv[2], "%x", &access_mask);

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Create domain user */

	result = rpccli_samr_CreateDomAlias(cli, mem_ctx,
					    &domain_pol,
					    &alias_name,
					    MAXIMUM_ALLOWED_ACCESS,
					    &alias_pol,
					    &rid);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &alias_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

 done:
	return result;
}

/* Lookup sam names */

static NTSTATUS cmd_samr_lookup_names(struct rpc_pipe_client *cli,
                                      TALLOC_CTX *mem_ctx,
                                      int argc, const char **argv)
{
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	POLICY_HND connect_pol, domain_pol;
	uint32 num_names;
	struct samr_Ids rids, name_types;
	int i;
	struct lsa_String *names = NULL;;

	if (argc < 3) {
		printf("Usage: %s  domain|builtin name1 [name2 [name3] [...]]\n", argv[0]);
		printf("check on the domain SID: S-1-5-21-x-y-z\n");
		printf("or check on the builtin SID: S-1-5-32\n");
		return NT_STATUS_OK;
	}

	/* Get sam policy and domain handles */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	if (StrCaseCmp(argv[1], "domain")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						&domain_sid,
						&domain_pol);
	else if (StrCaseCmp(argv[1], "builtin")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						CONST_DISCARD(struct dom_sid2 *, &global_sid_Builtin),
						&domain_pol);
	else
		return NT_STATUS_OK;

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Look up names */

	num_names = argc - 2;

	if ((names = TALLOC_ARRAY(mem_ctx, struct lsa_String, num_names)) == NULL) {
		rpccli_samr_Close(cli, mem_ctx, &domain_pol);
		rpccli_samr_Close(cli, mem_ctx, &connect_pol);
		result = NT_STATUS_NO_MEMORY;
		goto done;
	}

	for (i = 0; i < num_names; i++) {
		init_lsa_String(&names[i], argv[i + 2]);
	}

	result = rpccli_samr_LookupNames(cli, mem_ctx,
					 &domain_pol,
					 num_names,
					 names,
					 &rids,
					 &name_types);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Display results */

	for (i = 0; i < num_names; i++)
		printf("name %s: 0x%x (%d)\n", names[i].string, rids.ids[i],
		       name_types.ids[i]);

	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

/* Lookup sam rids */

static NTSTATUS cmd_samr_lookup_rids(struct rpc_pipe_client *cli,
                                     TALLOC_CTX *mem_ctx,
                                     int argc, const char **argv)
{
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	POLICY_HND connect_pol, domain_pol;
	uint32_t num_rids, *rids;
	struct lsa_Strings names;
	struct samr_Ids types;

	int i;

	if (argc < 3) {
		printf("Usage: %s domain|builtin rid1 [rid2 [rid3] [...]]\n", argv[0]);
		return NT_STATUS_OK;
	}

	/* Get sam policy and domain handles */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	if (StrCaseCmp(argv[1], "domain")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						&domain_sid,
						&domain_pol);
	else if (StrCaseCmp(argv[1], "builtin")==0)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						CONST_DISCARD(struct dom_sid2 *, &global_sid_Builtin),
						&domain_pol);
	else
		return NT_STATUS_OK;

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Look up rids */

	num_rids = argc - 2;

	if ((rids = TALLOC_ARRAY(mem_ctx, uint32, num_rids)) == NULL) {
		rpccli_samr_Close(cli, mem_ctx, &domain_pol);
		rpccli_samr_Close(cli, mem_ctx, &connect_pol);
		result = NT_STATUS_NO_MEMORY;
		goto done;
	}

	for (i = 0; i < argc - 2; i++)
                sscanf(argv[i + 2], "%i", &rids[i]);

	result = rpccli_samr_LookupRids(cli, mem_ctx,
					&domain_pol,
					num_rids,
					rids,
					&names,
					&types);

	if (!NT_STATUS_IS_OK(result) &&
	    !NT_STATUS_EQUAL(result, STATUS_SOME_UNMAPPED))
		goto done;

	/* Display results */

	for (i = 0; i < num_rids; i++) {
		printf("rid 0x%x: %s (%d)\n",
			rids[i], names.names[i].string, types.ids[i]);
	}

	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
 done:
	return result;
}

/* Delete domain group */

static NTSTATUS cmd_samr_delete_dom_group(struct rpc_pipe_client *cli,
                                         TALLOC_CTX *mem_ctx,
                                         int argc, const char **argv)
{
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	POLICY_HND connect_pol, domain_pol, group_pol;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s groupname\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc > 2)
                sscanf(argv[2], "%x", &access_mask);

	/* Get sam policy and domain handles */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					MAXIMUM_ALLOWED_ACCESS,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get handle on group */

	{
		struct samr_Ids group_rids, name_types;
		struct lsa_String lsa_acct_name;

		init_lsa_String(&lsa_acct_name, argv[1]);

		result = rpccli_samr_LookupNames(cli, mem_ctx,
						 &domain_pol,
						 1,
						 &lsa_acct_name,
						 &group_rids,
						 &name_types);
		if (!NT_STATUS_IS_OK(result))
			goto done;

		result = rpccli_samr_OpenGroup(cli, mem_ctx,
					       &domain_pol,
					       access_mask,
					       group_rids.ids[0],
					       &group_pol);

		if (!NT_STATUS_IS_OK(result))
			goto done;
	}

	/* Delete group */

	result = rpccli_samr_DeleteDomainGroup(cli, mem_ctx,
					       &group_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Display results */

	rpccli_samr_Close(cli, mem_ctx, &group_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);

 done:
	return result;
}

/* Delete domain user */

static NTSTATUS cmd_samr_delete_dom_user(struct rpc_pipe_client *cli,
                                         TALLOC_CTX *mem_ctx,
                                         int argc, const char **argv)
{
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	POLICY_HND connect_pol, domain_pol, user_pol;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;

	if ((argc < 2) || (argc > 3)) {
		printf("Usage: %s username\n", argv[0]);
		return NT_STATUS_OK;
	}

	if (argc > 2)
                sscanf(argv[2], "%x", &access_mask);

	/* Get sam policy and domain handles */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					MAXIMUM_ALLOWED_ACCESS,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get handle on user */

	{
		struct samr_Ids user_rids, name_types;
		struct lsa_String lsa_acct_name;

		init_lsa_String(&lsa_acct_name, argv[1]);

		result = rpccli_samr_LookupNames(cli, mem_ctx,
						 &domain_pol,
						 1,
						 &lsa_acct_name,
						 &user_rids,
						 &name_types);

		if (!NT_STATUS_IS_OK(result))
			goto done;

		result = rpccli_samr_OpenUser(cli, mem_ctx,
					      &domain_pol,
					      access_mask,
					      user_rids.ids[0],
					      &user_pol);

		if (!NT_STATUS_IS_OK(result))
			goto done;
	}

	/* Delete user */

	result = rpccli_samr_DeleteUser(cli, mem_ctx,
					&user_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Display results */

	rpccli_samr_Close(cli, mem_ctx, &user_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);

 done:
	return result;
}

/**********************************************************************
 * Query user security object
 */
static NTSTATUS cmd_samr_query_sec_obj(struct rpc_pipe_client *cli,
                                    TALLOC_CTX *mem_ctx,
                                    int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, user_pol, *pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 sec_info = DACL_SECURITY_INFORMATION;
	uint32 user_rid = 0;
	TALLOC_CTX *ctx = NULL;
	SEC_DESC_BUF *sec_desc_buf=NULL;
	bool domain = False;

	ctx=talloc_init("cmd_samr_query_sec_obj");

	if ((argc < 1) || (argc > 3)) {
		printf("Usage: %s [rid|-d] [sec_info]\n", argv[0]);
		printf("\tSpecify rid for security on user, -d for security on domain\n");
		talloc_destroy(ctx);
		return NT_STATUS_OK;
	}

	if (argc > 1) {
		if (strcmp(argv[1], "-d") == 0)
			domain = True;
		else
			sscanf(argv[1], "%i", &user_rid);
	}

	if (argc == 3) {
		sec_info = atoi(argv[2]);
	}

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	if (domain || user_rid)
		result = rpccli_samr_OpenDomain(cli, mem_ctx,
						&connect_pol,
						MAXIMUM_ALLOWED_ACCESS,
						&domain_sid,
						&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	if (user_rid)
		result = rpccli_samr_OpenUser(cli, mem_ctx,
					      &domain_pol,
					      MAXIMUM_ALLOWED_ACCESS,
					      user_rid,
					      &user_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Pick which query pol to use */

	pol = &connect_pol;

	if (domain)
		pol = &domain_pol;

	if (user_rid)
		pol = &user_pol;

	/* Query SAM security object */

	result = rpccli_samr_QuerySecurity(cli, mem_ctx,
					   pol,
					   sec_info,
					   &sec_desc_buf);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	display_sec_desc(sec_desc_buf->sd);

	rpccli_samr_Close(cli, mem_ctx, &user_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
done:
	talloc_destroy(ctx);
	return result;
}

static NTSTATUS cmd_samr_get_usrdom_pwinfo(struct rpc_pipe_client *cli,
					   TALLOC_CTX *mem_ctx,
					   int argc, const char **argv)
{
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	POLICY_HND connect_pol, domain_pol, user_pol;
	struct samr_PwInfo info;
	uint32_t rid;

	if (argc != 2) {
		printf("Usage: %s rid\n", argv[0]);
		return NT_STATUS_OK;
	}

	sscanf(argv[1], "%i", &rid);

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					MAXIMUM_ALLOWED_ACCESS,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	result = rpccli_samr_OpenUser(cli, mem_ctx,
				      &domain_pol,
				      MAXIMUM_ALLOWED_ACCESS,
				      rid,
				      &user_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	result = rpccli_samr_GetUserPwInfo(cli, mem_ctx, &user_pol, &info);
	if (NT_STATUS_IS_OK(result)) {
		printf("min_password_length: %d\n", info.min_password_length);
		printf("%s\n",
			NDR_PRINT_STRUCT_STRING(mem_ctx,
				samr_PasswordProperties, &info.password_properties));
	}

 done:
	rpccli_samr_Close(cli, mem_ctx, &user_pol);
	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);

	return result;
}

static NTSTATUS cmd_samr_get_dom_pwinfo(struct rpc_pipe_client *cli,
					TALLOC_CTX *mem_ctx,
					int argc, const char **argv)
{
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	struct lsa_String domain_name;
	struct samr_PwInfo info;

	if (argc < 1 || argc > 3) {
		printf("Usage: %s <domain>\n", argv[0]);
		return NT_STATUS_OK;
	}

	init_lsa_String(&domain_name, argv[1]);

	result = rpccli_samr_GetDomPwInfo(cli, mem_ctx, &domain_name, &info);

	if (NT_STATUS_IS_OK(result)) {
		printf("min_password_length: %d\n", info.min_password_length);
		display_password_properties(info.password_properties);
	}

	return result;
}

/* Look up domain name */

static NTSTATUS cmd_samr_lookup_domain(struct rpc_pipe_client *cli,
				       TALLOC_CTX *mem_ctx,
				       int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	fstring sid_string;
	struct lsa_String domain_name;
	DOM_SID *sid = NULL;

	if (argc != 2) {
		printf("Usage: %s domain_name\n", argv[0]);
		return NT_STATUS_OK;
	}

	init_lsa_String(&domain_name, argv[1]);

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  access_mask,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_LookupDomain(cli, mem_ctx,
					  &connect_pol,
					  &domain_name,
					  &sid);

	if (NT_STATUS_IS_OK(result)) {
		sid_to_fstring(sid_string, sid);
		printf("SAMR_LOOKUP_DOMAIN: Domain Name: %s Domain SID: %s\n",
		       argv[1], sid_string);
	}

	rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	rpccli_samr_Close(cli, mem_ctx, &connect_pol);
done:
	return result;
}

/* Change user password */

static NTSTATUS cmd_samr_chgpasswd(struct rpc_pipe_client *cli,
				   TALLOC_CTX *mem_ctx,
				   int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol, user_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	const char *user, *oldpass, *newpass;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	struct samr_Ids rids, types;
	struct lsa_String lsa_acct_name;

	if (argc < 3) {
		printf("Usage: %s username oldpass newpass\n", argv[0]);
		return NT_STATUS_INVALID_PARAMETER;
	}

	user = argv[1];
	oldpass = argv[2];
	newpass = argv[3];

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	init_lsa_String(&lsa_acct_name, user);

	result = rpccli_samr_LookupNames(cli, mem_ctx,
					 &domain_pol,
					 1,
					 &lsa_acct_name,
					 &rids,
					 &types);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	result = rpccli_samr_OpenUser(cli, mem_ctx,
				      &domain_pol,
				      access_mask,
				      rids.ids[0],
				      &user_pol);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

	/* Change user password */
	result = rpccli_samr_chgpasswd_user(cli, mem_ctx,
					    &user_pol,
					    newpass,
					    oldpass);

	if (!NT_STATUS_IS_OK(result)) {
		goto done;
	}

 done:
	if (is_valid_policy_hnd(&user_pol)) {
		rpccli_samr_Close(cli, mem_ctx, &user_pol);
	}
	if (is_valid_policy_hnd(&domain_pol)) {
		rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	}
	if (is_valid_policy_hnd(&connect_pol)) {
		rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	}

	return result;
}


/* Change user password */

static NTSTATUS cmd_samr_chgpasswd2(struct rpc_pipe_client *cli,
				    TALLOC_CTX *mem_ctx,
				    int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	const char *user, *oldpass, *newpass;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;

	if (argc < 3) {
		printf("Usage: %s username oldpass newpass\n", argv[0]);
		return NT_STATUS_INVALID_PARAMETER;
	}

	user = argv[1];
	oldpass = argv[2];
	newpass = argv[3];

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Change user password */
	result = rpccli_samr_chgpasswd_user2(cli, mem_ctx, user, newpass, oldpass);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

 done:
	return result;
}


/* Change user password */

static NTSTATUS cmd_samr_chgpasswd3(struct rpc_pipe_client *cli,
				    TALLOC_CTX *mem_ctx,
				    int argc, const char **argv)
{
	POLICY_HND connect_pol, domain_pol;
	NTSTATUS result = NT_STATUS_UNSUCCESSFUL;
	const char *user, *oldpass, *newpass;
	uint32 access_mask = MAXIMUM_ALLOWED_ACCESS;
	struct samr_DomInfo1 *info = NULL;
	struct samr_ChangeReject *reject = NULL;

	if (argc < 3) {
		printf("Usage: %s username oldpass newpass\n", argv[0]);
		return NT_STATUS_INVALID_PARAMETER;
	}

	user = argv[1];
	oldpass = argv[2];
	newpass = argv[3];

	/* Get sam policy handle */

	result = rpccli_try_samr_connects(cli, mem_ctx,
					  MAXIMUM_ALLOWED_ACCESS,
					  &connect_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Get domain policy handle */

	result = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_pol,
					access_mask,
					&domain_sid,
					&domain_pol);

	if (!NT_STATUS_IS_OK(result))
		goto done;

	/* Change user password */
	result = rpccli_samr_chgpasswd_user3(cli, mem_ctx,
					     user,
					     newpass,
					     oldpass,
					     &info,
					     &reject);

	if (NT_STATUS_EQUAL(result, NT_STATUS_PASSWORD_RESTRICTION)) {

		display_sam_dom_info_1(info);

		switch (reject->reason) {
			case SAMR_REJECT_TOO_SHORT:
				d_printf("SAMR_REJECT_TOO_SHORT\n");
				break;
			case SAMR_REJECT_IN_HISTORY:
				d_printf("SAMR_REJECT_IN_HISTORY\n");
				break;
			case SAMR_REJECT_COMPLEXITY:
				d_printf("SAMR_REJECT_COMPLEXITY\n");
				break;
			case SAMR_REJECT_OTHER:
				d_printf("SAMR_REJECT_OTHER\n");
				break;
			default:
				d_printf("unknown reject reason: %d\n",
					reject->reason);
				break;
		}
	}

	if (!NT_STATUS_IS_OK(result))
		goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &domain_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

	result = rpccli_samr_Close(cli, mem_ctx, &connect_pol);
	if (!NT_STATUS_IS_OK(result)) goto done;

 done:
	return result;
}

static NTSTATUS cmd_samr_get_dispinfo_idx(struct rpc_pipe_client *cli,
					  TALLOC_CTX *mem_ctx,
					  int argc, const char **argv)
{
	NTSTATUS status;
	struct policy_handle connect_handle;
	struct policy_handle domain_handle;
	uint16_t level = 1;
	struct lsa_String name;
	uint32_t idx = 0;

	if (argc < 2 || argc > 3) {
		printf("Usage: %s name level\n", argv[0]);
		return NT_STATUS_INVALID_PARAMETER;
	}

	init_lsa_String(&name, argv[1]);

	if (argc == 3) {
		level = atoi(argv[2]);
	}

	status = rpccli_try_samr_connects(cli, mem_ctx,
					  SEC_RIGHTS_MAXIMUM_ALLOWED,
					  &connect_handle);

	if (!NT_STATUS_IS_OK(status)) {
		goto done;
	}

	status = rpccli_samr_OpenDomain(cli, mem_ctx,
					&connect_handle,
					SEC_RIGHTS_MAXIMUM_ALLOWED,
					&domain_sid,
					&domain_handle);

	if (!NT_STATUS_IS_OK(status))
		goto done;


	status = rpccli_samr_GetDisplayEnumerationIndex(cli, mem_ctx,
							&domain_handle,
							level,
							&name,
							&idx);

	if (NT_STATUS_IS_OK(status) ||
	    NT_STATUS_EQUAL(status, NT_STATUS_NO_MORE_ENTRIES)) {
		printf("idx: %d (0x%08x)\n", idx, idx);
	}
 done:

	if (is_valid_policy_hnd(&domain_handle)) {
		rpccli_samr_Close(cli, mem_ctx, &domain_handle);
	}
	if (is_valid_policy_hnd(&connect_handle)) {
		rpccli_samr_Close(cli, mem_ctx, &connect_handle);
	}

	return status;

}
/* List of commands exported by this module */

struct cmd_set samr_commands[] = {

	{ "SAMR" },

	{ "queryuser", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_user, 		NULL, &ndr_table_samr.syntax_id, NULL,	"Query user info",         "" },
	{ "querygroup", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_group, 		NULL, &ndr_table_samr.syntax_id, NULL,	"Query group info",        "" },
	{ "queryusergroups", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_usergroups, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query user groups",       "" },
	{ "queryuseraliases", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_useraliases, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query user aliases",      "" },
	{ "querygroupmem", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_groupmem, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query group membership",  "" },
	{ "queryaliasmem", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_aliasmem, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query alias membership",  "" },
	{ "queryaliasinfo", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_aliasinfo, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query alias info",       "" },
	{ "deletealias", 	RPC_RTYPE_NTSTATUS, cmd_samr_delete_alias, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Delete an alias",  "" },
	{ "querydispinfo", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_dispinfo, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query display info",      "" },
	{ "querydispinfo2", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_dispinfo2, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query display info",      "" },
	{ "querydispinfo3", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_dispinfo3, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query display info",      "" },
	{ "querydominfo", 	RPC_RTYPE_NTSTATUS, cmd_samr_query_dominfo, 	NULL, &ndr_table_samr.syntax_id, NULL,	"Query domain info",       "" },
	{ "enumdomusers",       RPC_RTYPE_NTSTATUS, cmd_samr_enum_dom_users,       NULL, &ndr_table_samr.syntax_id, NULL,	"Enumerate domain users", "" },
	{ "enumdomgroups",      RPC_RTYPE_NTSTATUS, cmd_samr_enum_dom_groups,       NULL, &ndr_table_samr.syntax_id, NULL,	"Enumerate domain groups", "" },
	{ "enumalsgroups",      RPC_RTYPE_NTSTATUS, cmd_samr_enum_als_groups,       NULL, &ndr_table_samr.syntax_id, NULL,	"Enumerate alias groups",  "" },
	{ "enumdomains",        RPC_RTYPE_NTSTATUS, cmd_samr_enum_domains,          NULL, &ndr_table_samr.syntax_id, NULL,	"Enumerate domains",  "" },

	{ "createdomuser",      RPC_RTYPE_NTSTATUS, cmd_samr_create_dom_user,       NULL, &ndr_table_samr.syntax_id, NULL,	"Create domain user",      "" },
	{ "createdomgroup",     RPC_RTYPE_NTSTATUS, cmd_samr_create_dom_group,      NULL, &ndr_table_samr.syntax_id, NULL,	"Create domain group",     "" },
	{ "createdomalias",     RPC_RTYPE_NTSTATUS, cmd_samr_create_dom_alias,      NULL, &ndr_table_samr.syntax_id, NULL,	"Create domain alias",     "" },
	{ "samlookupnames",     RPC_RTYPE_NTSTATUS, cmd_samr_lookup_names,          NULL, &ndr_table_samr.syntax_id, NULL,	"Look up names",           "" },
	{ "samlookuprids",      RPC_RTYPE_NTSTATUS, cmd_samr_lookup_rids,           NULL, &ndr_table_samr.syntax_id, NULL,	"Look up names",           "" },
	{ "deletedomgroup",     RPC_RTYPE_NTSTATUS, cmd_samr_delete_dom_group,      NULL, &ndr_table_samr.syntax_id, NULL,	"Delete domain group",     "" },
	{ "deletedomuser",      RPC_RTYPE_NTSTATUS, cmd_samr_delete_dom_user,       NULL, &ndr_table_samr.syntax_id, NULL,	"Delete domain user",      "" },
	{ "samquerysecobj",     RPC_RTYPE_NTSTATUS, cmd_samr_query_sec_obj,         NULL, &ndr_table_samr.syntax_id, NULL, "Query SAMR security object",   "" },
	{ "getdompwinfo",       RPC_RTYPE_NTSTATUS, cmd_samr_get_dom_pwinfo,        NULL, &ndr_table_samr.syntax_id, NULL, "Retrieve domain password info", "" },
	{ "getusrdompwinfo",    RPC_RTYPE_NTSTATUS, cmd_samr_get_usrdom_pwinfo,     NULL, &ndr_table_samr.syntax_id, NULL, "Retrieve user domain password info", "" },

	{ "lookupdomain",       RPC_RTYPE_NTSTATUS, cmd_samr_lookup_domain,         NULL, &ndr_table_samr.syntax_id, NULL, "Lookup Domain Name", "" },
	{ "chgpasswd",          RPC_RTYPE_NTSTATUS, cmd_samr_chgpasswd,             NULL, &ndr_table_samr.syntax_id, NULL, "Change user password", "" },
	{ "chgpasswd2",         RPC_RTYPE_NTSTATUS, cmd_samr_chgpasswd2,            NULL, &ndr_table_samr.syntax_id, NULL, "Change user password", "" },
	{ "chgpasswd3",         RPC_RTYPE_NTSTATUS, cmd_samr_chgpasswd3,            NULL, &ndr_table_samr.syntax_id, NULL, "Change user password", "" },
	{ "getdispinfoidx",     RPC_RTYPE_NTSTATUS, cmd_samr_get_dispinfo_idx,      NULL, &ndr_table_samr.syntax_id, NULL, "Get Display Information Index", "" },
	{ NULL }
};
