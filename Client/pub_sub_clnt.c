/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "pub_sub.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

short *
set_channel_1(argp, clnt)
	param *argp;
	CLIENT *clnt;
{
	static short clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call(clnt, set_channel, xdr_param, argp, xdr_short, &clnt_res, TIMEOUT) != RPC_SUCCESS)
		return (NULL);
	return (&clnt_res);
}

short *
subscribe_1(argp, clnt)
	param *argp;
	CLIENT *clnt;
{
	static short clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call(clnt, subscribe, xdr_param, argp, xdr_short, &clnt_res, TIMEOUT) != RPC_SUCCESS)
		return (NULL);
	return (&clnt_res);
}

short *
unsubscribe_1(argp, clnt)
	param *argp;
	CLIENT *clnt;
{
	static short clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call(clnt, unsubscribe, xdr_param, argp, xdr_short, &clnt_res, TIMEOUT) != RPC_SUCCESS)
		return (NULL);
	return (&clnt_res);
}

short *
publish_1(argp, clnt)
	param *argp;
	CLIENT *clnt;
{
	static short clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call(clnt, publish, xdr_param, argp, xdr_short, &clnt_res, TIMEOUT) != RPC_SUCCESS)
		return (NULL);
	return (&clnt_res);
}

sessionid *
get_session_1(argp, clnt)
	user *argp;
	CLIENT *clnt;
{
	static sessionid clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call(clnt, get_session, xdr_user, argp, xdr_sessionid, &clnt_res, TIMEOUT) != RPC_SUCCESS)
		return (NULL);
	return (&clnt_res);
}

short *
validate_1(argp, clnt)
	param *argp;
	CLIENT *clnt;
{
	static short clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call(clnt, validate, xdr_param, argp, xdr_short, &clnt_res, TIMEOUT) != RPC_SUCCESS)
		return (NULL);
	return (&clnt_res);
}

short *
invalidate_1(argp, clnt)
	sessionid *argp;
	CLIENT *clnt;
{
	static short clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call(clnt, invalidate, xdr_sessionid, argp, xdr_short, &clnt_res, TIMEOUT) != RPC_SUCCESS)
		return (NULL);
	return (&clnt_res);
}
