//
//  mgw.h
//
//  Created by jl777 2014, refactored MGW
//  Copyright (c) 2014 jl777. MIT License.
//

#ifndef mgw_h
#define mgw_h

#define DEPOSIT_XFER_DURATION (13/13)
#define MIN_DEPOSIT_FACTOR 5

#define GET_COINDEPOSIT_ADDRESS 'g'
#define BIND_DEPOSIT_ADDRESS 'b'
#define DEPOSIT_CONFIRMED 'd'
#define MONEY_SENT 'm'

int32_t is_limbo_redeem(char *coinstr,char *txid)
{
    uint64_t nxt64bits;
    int32_t j,skipflag = 0;
    struct coin_info *cp;
    if ( (cp= get_coin_info(coinstr)) != 0 && cp->limboarray != 0 )
    {
        nxt64bits = calc_nxt64bits(txid);
        for (j=0; cp->limboarray[j]!=0; j++)
            if ( nxt64bits == cp->limboarray[j] )
            {
                printf(">>>>>>>>>>> j.%d found limbotx %llu\n",j,(long long)cp->limboarray[j]);
                skipflag = 1;
                break;
            }
    }
    return(skipflag);
}

static int32_t _cmp_vps(const void *a,const void *b)
{
#define vp_a (*(struct coin_txidind **)a)
#define vp_b (*(struct coin_txidind **)b)
	if ( vp_b->value > vp_a->value )
		return(1);
	else if ( vp_b->value < vp_a->value )
		return(-1);
	return(0);
#undef vp_a
#undef vp_b
}

void sort_vps(struct coin_txidind **vps,int32_t num)
{
	qsort(vps,num,sizeof(*vps),_cmp_vps);
}

void update_unspent_funds(struct coin_info *cp,struct coin_txidind *cointp,int32_t n)
{
    struct unspent_info *up = &cp->unspent;
    uint64_t value;
    if ( n > up->maxvps )
    {
        up->vps = realloc(up->vps,n * sizeof(*up->vps));
        up->maxvps = n;
    }
    if ( cointp == 0 )
    {
        up->num = 0;
        up->maxvp = up->minvp = 0;
        memset(up->vps,0,up->maxvps * sizeof(*up->vps));
        up->maxunspent = up->unspent = up->maxavail = up->minavail = 0;
    }
    else
    {
        value = cointp->value;
        up->maxunspent += value;
        if ( cointp->entry.spent == 0 )
        {
            up->vps[up->num++] = cointp;
            up->unspent += value;
            if ( value > up->maxavail )
            {
                up->maxvp = cointp;
                up->maxavail = value;
            }
            if ( up->minavail == 0 || value < up->minavail )
            {
                up->minavail = value;
                up->minvp = cointp;
            }
        }
    }
}

struct multisig_addr *find_msigaddr(char *msigaddr)
{
    int32_t createdflag;
    return(MTadd_hashtable(&createdflag,&SuperNET_dbs[MULTISIG_DATA].ramtable,msigaddr));
    //return((struct multisig_addr *)find_storage(MULTISIG_DATA,msigaddr,0));
}

int32_t update_msig_info(struct multisig_addr *msig,int32_t syncflag)
{
    DBT key,data,*datap;
    int32_t ret,createdflag;
    struct multisig_addr *msigram;
    struct SuperNET_db *sdb = &SuperNET_dbs[MULTISIG_DATA];
    if ( msig == 0 && syncflag != 0 )
        return(dbsync(sdb,0));
    if ( msig->H.size == 0 )
        msig->H.size = sizeof(*msig) + (msig->n * sizeof(msig->pubkeys[0]));
    msigram = MTadd_hashtable(&createdflag,&sdb->ramtable,msig->multisigaddr);
    if ( msigram->created != 0 && msig->created != 0 )
    {
        if ( msigram->created < msig->created )
            msig->created = msigram->created;
        else msigram->created = msig->created;
    }
    else if ( msig->created == 0 )
        msig->created = msigram->created;
    //if ( msigram->sender == 0 && msig->sender != 0 )
    //    createdflag = 1;
    if ( createdflag != 0 )//|| memcmp(msigram,msig,msig->H.size) != 0 )
    {
        clear_pair(&key,&data);
        key.data = msig->multisigaddr;
        key.size = (int32_t)(strlen(msig->multisigaddr) + 1);
        data.size = msig->H.size;
        if ( sdb->overlap_write != 0 )
        {
            data.data = calloc(1,msig->H.size);
            memcpy(data.data,msig,msig->H.size);
            datap = calloc(1,sizeof(*datap));
            *datap = data;
        }
        else
        {
            data.data = msig;
            datap = &data;
        }
        printf("add (%s)\n",msig->multisigaddr);
        if ( (ret= dbput(sdb,0,&key,datap,0)) != 0 )
            Storage->err(Storage,ret,"Database put for quote failed.");
        else if ( syncflag != 0 ) ret = dbsync(sdb,0);
    } return(1);
    return(ret);
}

struct multisig_addr *alloc_multisig_addr(char *coinstr,int32_t m,int32_t n,char *NXTaddr,char *sender)
{
    struct multisig_addr *msig;
    int32_t size = (int32_t)(sizeof(*msig) + n*sizeof(struct pubkey_info));
    msig = calloc(1,size);
    msig->H.size = size;
    msig->n = n;
    msig->created = (uint32_t)time(NULL);
    msig->sender = calc_nxt64bits(sender);
    safecopy(msig->coinstr,coinstr,sizeof(msig->coinstr));
    safecopy(msig->NXTaddr,NXTaddr,sizeof(msig->NXTaddr));
    msig->m = m;
    return(msig);
}

long calc_pubkey_jsontxt(char *jsontxt,struct pubkey_info *ptr,char *postfix)
{
    sprintf(jsontxt,"{\"address\":\"%s\",\"pubkey\":\"%s\",\"srv\":\"%llu\"}%s",ptr->coinaddr,ptr->pubkey,(long long)ptr->nxt64bits,postfix);//\"ipaddr\":\"%s\"ptr->server,
    return(strlen(jsontxt));
}

char *create_multisig_json(struct multisig_addr *msig)
{
    long i,len = 0;
    char jsontxt[65536],pubkeyjsontxt[65536];
    for (i=0; i<msig->n; i++)
        len += calc_pubkey_jsontxt(pubkeyjsontxt,&msig->pubkeys[i],(i<(msig->n - 1)) ? "," : "");
    sprintf(jsontxt,"{\"sender\":\"%llu\",\"created\":%u,\"M\":%d,\"N\":%d,\"NXTaddr\":\"%s\",\"address\":\"%s\",\"redeemScript\":\"%s\",\"coin\":\"%s\",\"coinid\":\"%d\",\"pubkey\":[%s]}",(long long)msig->sender,msig->created,msig->m,msig->n,msig->NXTaddr,msig->multisigaddr,msig->redeemScript,msig->coinstr,conv_coinstr(msig->coinstr),pubkeyjsontxt);
    printf("(%s) pubkeys len.%ld msigjsonlen.%ld\n",jsontxt,len,strlen(jsontxt));
    return(clonestr(jsontxt));
}

char *createmultisig_json_params(struct multisig_addr *msig,char *acctparm)
{
    int32_t i;
    char *paramstr = 0;
    cJSON *array,*mobj,*keys,*key;
    keys = cJSON_CreateArray();
    for (i=0; i<msig->n; i++)
    {
        key = cJSON_CreateString(msig->pubkeys[i].pubkey);
        cJSON_AddItemToArray(keys,key);
    }
    mobj = cJSON_CreateNumber(msig->m);
    array = cJSON_CreateArray();
    if ( array != 0 )
    {
        cJSON_AddItemToArray(array,mobj);
        cJSON_AddItemToArray(array,keys);
        if ( acctparm != 0 )
            cJSON_AddItemToArray(array,cJSON_CreateString(acctparm));
        paramstr = cJSON_Print(array);
        free_json(array);
    }
    //printf("createmultisig_json_params.%s\n",paramstr);
    return(paramstr);
}

int32_t replace_msig_json(int32_t replaceflag,char *refNXTaddr,char *acctcoinaddr,char *pubkey,char *coinstr,char *jsonstr)
{
    cJSON *array,*item,*json;
    int32_t flag,n,i = -1;
    char coin[1024],refaddr[1024],*str;
    flag = 0;
    if ( (json= cJSON_Parse(jsonstr)) != 0 )
    {
        array = cJSON_GetObjectItem(json,"coins");
        if ( array != 0 && is_cJSON_Array(array) != 0 && (n= cJSON_GetArraySize(array)) >= 0 )
        {
            for (i=0; i<n; i++)
            {
                item = cJSON_GetArrayItem(array,i);
                copy_cJSON(coin,cJSON_GetObjectItem(item,"coin"));
                copy_cJSON(refaddr,cJSON_GetObjectItem(item,"NXTaddr"));
                if ( strcmp(coin,coinstr) == 0 && strcmp(refaddr,refNXTaddr) == 0 )
                {
                    if ( replaceflag != 0 )
                    {
                        ensure_jsonitem(item,"pubkey",pubkey);
                        ensure_jsonitem(item,"addr",acctcoinaddr);
                        flag = 1;
                    }
                    else
                    {
                        copy_cJSON(pubkey,cJSON_GetObjectItem(item,"pubkey"));
                        copy_cJSON(acctcoinaddr,cJSON_GetObjectItem(item,"addr"));
                    }
                    break;
                }
            }
            if ( i == n )
                i = -1;
        }
        if ( replaceflag != 0 )
        {
            if ( flag == 0 )
            {
                item = cJSON_CreateObject();
                ensure_jsonitem(item,"coin",coinstr);
                ensure_jsonitem(item,"NXTaddr",refNXTaddr);
                ensure_jsonitem(item,"address",acctcoinaddr);
                ensure_jsonitem(item,"pubkey",pubkey);
                if ( array != 0 )
                    cJSON_AddItemToArray(array,item);
                else
                {
                    array = cJSON_CreateArray();
                    cJSON_AddItemToArray(array,item);
                    cJSON_AddItemToObject(json,"coins",array);
                }
            }
            str = cJSON_Print(json);
            stripwhite_ns(str,strlen(str));
            strcpy(jsonstr,str);
            free(str);
        }
        free_json(json);
        return(i);
    }
    if ( replaceflag != 0 && flag == 0 )
    {
        //sprintf(jsontxt,"{\"created\":%u,\"M\":%d,\"N\":%d,\"NXTaddr\":\"%s\",\"address\":\"%s\",\"redeemScript\":\"%s\",\"coin\":\"%s\",\"coinid\":\"%d\",\"pubkey\":[%s]}",msig->created,msig->m,msig->n,msig->NXTaddr,msig->multisigaddr,msig->redeemScript,msig->coinstr,conv_coinstr(msig->coinstr),pubkeyjsontxt);
        sprintf(jsonstr,"{\"coins\":[{\"coin\":\"%s\",\"NXTaddr\":\"%s\",\"address\":\"%s\",\"pubkey\":\"%s\"}]}",coinstr,refNXTaddr,acctcoinaddr,pubkey);
        if ( (json= cJSON_Parse(jsonstr)) != 0 )
            free_json(json);
        else printf("PARSEERROR.(%s)\n",jsonstr);
        return(0);
    }
    return(-1);
}

int32_t issue_createmultisig(struct coin_info *cp,struct multisig_addr *msig)
{
    int32_t flag = 0;
    char addr[256];
    cJSON *json,*msigobj,*redeemobj;
    char *params,*retstr = 0;
    params = createmultisig_json_params(msig,0);
    flag = 0;
    if ( params != 0 )
    {
        printf("multisig params.(%s)\n",params);
        if ( cp->use_addmultisig != 0 )
        {
            retstr = bitcoind_RPC(0,cp->name,cp->serverport,cp->userpass,"addmultisigaddress",params);
            if ( retstr != 0 )
            {
                strcpy(msig->multisigaddr,retstr);
                free(retstr);
                sprintf(addr,"\"%s\"",msig->multisigaddr);
                retstr = bitcoind_RPC(0,cp->name,cp->serverport,cp->userpass,"validateaddress",addr);
                if ( retstr != 0 )
                {
                    json = cJSON_Parse(retstr);
                    if ( json == 0 ) printf("Error before: [%s]\n",cJSON_GetErrorPtr());
                    else
                    {
                        if ( (redeemobj= cJSON_GetObjectItem(json,"hex")) != 0 )
                        {
                            copy_cJSON(msig->redeemScript,redeemobj);
                            flag = 1;
                        } else printf("missing redeemScript in (%s)\n",retstr);
                        free_json(json);
                    }
                    free(retstr);
                }
            } else printf("error creating multisig address\n");
        }
        else
        {
            retstr = bitcoind_RPC(0,cp->name,cp->serverport,cp->userpass,"createmultisig",params);
            if ( retstr != 0 )
            {
                json = cJSON_Parse(retstr);
                if ( json == 0 ) printf("Error before: [%s]\n",cJSON_GetErrorPtr());
                else
                {
                    if ( (msigobj= cJSON_GetObjectItem(json,"address")) != 0 )
                    {
                        if ( (redeemobj= cJSON_GetObjectItem(json,"redeemScript")) != 0 )
                        {
                            copy_cJSON(msig->multisigaddr,msigobj);
                            copy_cJSON(msig->redeemScript,redeemobj);
                            flag = 1;
                        } else printf("missing redeemScript in (%s)\n",retstr);
                    } else printf("multisig missing address in (%s) params.(%s)\n",retstr,params);
                    printf("addmultisig.(%s)\n",retstr);
                    free_json(json);
                }
                free(retstr);
            } else printf("error issuing createmultisig.(%s)\n",params);
        }
        free(params);
    } else printf("error generating msig params\n");
    return(flag);
}

struct multisig_addr *gen_multisig_addr(char *sender,int32_t M,int32_t N,struct coin_info *cp,char *refNXTaddr,struct contact_info **contacts)
{
    int32_t i,j,flag = 0;
    char acctcoinaddr[128],pubkey[1024];
    struct contact_info *contact;
    struct multisig_addr *msig;
    if ( cp == 0 )
        return(0);
    msig = alloc_multisig_addr(cp->name,M,N,refNXTaddr,sender);
    for (i=0; i<N; i++)
    {
        flag = 0;
        if ( (contact= contacts[i]) != 0 && (contact->jsonstr[0] == '[' || contact->jsonstr[0] == '{') )
        {
            acctcoinaddr[0] = pubkey[0] = 0;
            if ( (j= replace_msig_json(0,refNXTaddr,acctcoinaddr,pubkey,cp->name,contact->jsonstr)) >= 0 )
            {
                strcpy(msig->pubkeys[j].coinaddr,acctcoinaddr);
                strcpy(msig->pubkeys[j].pubkey,pubkey);
                msig->pubkeys[j].nxt64bits = contact->nxt64bits;
            }
            /*if ( flag == 0 )
             {
             free(msig);
             return(0);
             }*/
        }
    }
    flag = issue_createmultisig(cp,msig);
    if ( flag == 0 )
    {
        free(msig);
        return(0);
    }
    return(msig);
}

char *genmultisig(char *NXTaddr,char *NXTACCTSECRET,char *previpaddr,char *coinstr,char *refacct,int32_t M,int32_t N,struct contact_info **contacts,int32_t n)
{
    struct coin_info *cp = get_coin_info(coinstr);
    struct multisig_addr *msig,*dbmsig;
    struct contact_info *contact,*refcontact = 0;
    char refNXTaddr[64],hopNXTaddr[64],destNXTaddr[64],pubkey[1024],acctcoinaddr[128],buf[1024],*retstr = 0;
    int32_t i,valid = 0;
    refNXTaddr[0] = 0;
    if ( (refcontact= find_contact(refacct)) != 0 )
    {
        if ( refcontact->nxt64bits != 0 )
            expand_nxt64bits(refNXTaddr,refcontact->nxt64bits);
    }
    if ( refNXTaddr[0] == 0 )
        return(clonestr("\"error\":\"genmultisig couldnt find refcontact\"}"));
    for (i=0; i<n; i++)
    {
        acctcoinaddr[0] = pubkey[0] = 0;
        if ( (contact= contacts[i]) != 0 && contact->nxt64bits != 0 )
        {
            if ( ismynxtbits(contact->nxt64bits) != 0 )
            {
                expand_nxt64bits(destNXTaddr,contact->nxt64bits);
                if ( cp != 0 && get_acct_coinaddr(acctcoinaddr,cp,destNXTaddr) != 0 && get_bitcoind_pubkey(pubkey,cp,acctcoinaddr) != 0 )
                {
                    valid += replace_msig_json(1,refNXTaddr,acctcoinaddr,pubkey,cp->name,contact->jsonstr);
                    update_contact_info(contact);
                }
                else printf("error getting msigaddr for cp.%p ref.(%s) addr.(%s) pubkey.(%s)\n",cp,destNXTaddr,acctcoinaddr,pubkey);
            }
            else
            {
                hopNXTaddr[0] = 0;
                sprintf(buf,"{\"requestType\":\"getmsigpubkey\",\"coin\":\"%s\",\"refNXTaddr\":\"%s\"}",coinstr,refNXTaddr);
                retstr = send_tokenized_cmd(hopNXTaddr,0,NXTaddr,NXTACCTSECRET,buf,destNXTaddr);
            }
        }
    }
    if ( valid == N )
    {
        if ( (msig= gen_multisig_addr(NXTaddr,M,N,cp,refacct,contacts)) != 0 )
        {
            if ( (dbmsig= find_msigaddr(msig->multisigaddr)) == 0 )
                update_msig_info(msig,1);
            else free(dbmsig);
            retstr = create_multisig_json(msig);
            free(msig);
        }
    }
    else
    {
        sprintf(buf,"{\"error\":\"missing msig info\",\"refacct\":\"%s\",\"coin\":\"%s\",\"M\":%d,\"N\":%d,\"valid\":%d}",refacct,coinstr,M,N,valid);
        retstr = clonestr(buf);
    }
    return(retstr);
}

struct multisig_addr *decode_msigjson(char *NXTaddr,cJSON *obj,char *sender)
{
    int32_t j,M,n,coinid;
    char nxtstr[512],coinstr[64],ipaddr[64];
    struct multisig_addr *msig = 0;
    cJSON *pobj,*redeemobj,*pubkeysobj,*addrobj,*nxtobj,*nameobj;
    coinid = (int)get_cJSON_int(obj,"coinid");
    nameobj = cJSON_GetObjectItem(obj,"coin");
    copy_cJSON(coinstr,nameobj);
    if ( coinstr[0] != 0 )
    {
        addrobj = cJSON_GetObjectItem(obj,"address");
        redeemobj = cJSON_GetObjectItem(obj,"redeemScript");
        pubkeysobj = cJSON_GetObjectItem(obj,"pubkey");
        nxtobj = cJSON_GetObjectItem(obj,"NXTaddr");
        if ( nxtobj != 0 )
        {
            copy_cJSON(nxtstr,nxtobj);
            if ( NXTaddr != 0 && strcmp(nxtstr,NXTaddr) != 0 )
                printf("WARNING: mismatched NXTaddr.%s vs %s\n",nxtstr,NXTaddr);
        }
        //printf("msig.%p %p %p %p\n",msig,addrobj,redeemobj,pubkeysobj);
        if ( nxtstr[0] != 0 && addrobj != 0 && redeemobj != 0 && pubkeysobj != 0 )
        {
            n = cJSON_GetArraySize(pubkeysobj);
            M = (int32_t)get_API_int(cJSON_GetObjectItem(obj,"M"),n-1);
            msig = alloc_multisig_addr(coinstr,M,n,nxtstr,sender);
            safecopy(msig->coinstr,coinstr,sizeof(msig->coinstr));
            copy_cJSON(msig->redeemScript,redeemobj);
            copy_cJSON(msig->multisigaddr,addrobj);
            for (j=0; j<n; j++)
            {
                pobj = cJSON_GetArrayItem(pubkeysobj,j);
                if ( pobj != 0 )
                {
                    copy_cJSON(msig->pubkeys[j].coinaddr,cJSON_GetObjectItem(pobj,"address"));
                    copy_cJSON(msig->pubkeys[j].pubkey,cJSON_GetObjectItem(pobj,"pubkey"));
                    msig->pubkeys[j].nxt64bits = get_API_nxt64bits(cJSON_GetObjectItem(pobj,"srv"));
                    copy_cJSON(ipaddr,cJSON_GetObjectItem(pobj,"ipaddr"));
                    //printf("ip%d.(%s) ",j,ipaddr);
                    if ( ipaddr[0] == 0 && j < 3 )
                        strcpy(ipaddr,Server_names[j]);
                    msig->pubkeys[j].ipbits = calc_ipbits(ipaddr);
                } else { free(msig); msig = 0; }
            }
        } else { printf("%p %p %p\n",addrobj,redeemobj,pubkeysobj); free(msig); msig = 0; }
        return(msig);
    }
    //printf("decode msig:  error parsing.(%s)\n",cJSON_Print(obj));
    return(0);
}

// network aware funcs
void publish_withdraw_info(struct coin_info *cp,struct batch_info *wp)
{
    struct batch_info W;
    int32_t gatewayid;
    wp->W.coinid = conv_coinstr(cp->name);
    if ( wp->W.coinid < 0 )
    {
        printf("unknown coin.(%s)\n",cp->name);
        return;
    }
    wp->W.srcgateway = Global_mp->gatewayid;
    for (gatewayid=0; gatewayid<NUM_GATEWAYS; gatewayid++)
    {
        wp->W.destgateway = gatewayid;
        W = *wp;
        fprintf(stderr,"publish_withdraw_info.%d -> %d coinid.%d %.8f crc %08x\n",Global_mp->gatewayid,gatewayid,wp->W.coinid,dstr(wp->W.amount),W.rawtx.batchcrc);
        if ( gatewayid == Global_mp->gatewayid )
            cp->withdrawinfos[gatewayid] = *wp;
        else if ( server_request(&Global_mp->gensocks[gatewayid],Server_names[gatewayid],&W.W.H,MULTIGATEWAY_VARIANT,MULTIGATEWAY_SYNCWITHDRAW) == sizeof(W) )
        {
            portable_mutex_lock(&cp->consensus_mutex);
            cp->withdrawinfos[gatewayid] = W;
            portable_mutex_unlock(&cp->consensus_mutex);
        }
        fprintf(stderr,"got publish_withdraw_info.%d -> %d coinid.%d %.8f crc %08x\n",Global_mp->gatewayid,gatewayid,wp->W.coinid,dstr(wp->W.amount),cp->withdrawinfos[gatewayid].rawtx.batchcrc);
    }
}

int32_t process_directnet_syncwithdraw(struct batch_info *wp,char *clientip)
{
    int32_t gatewayid;
    struct coin_info *cp;
    if ( (cp= get_coin_info(coinid_str(wp->W.coinid))) == 0 )
        printf("cant find coinid.%d\n",wp->W.coinid);
    else
    {
        gatewayid = (wp->W.srcgateway % NUM_GATEWAYS);
        cp->withdrawinfos[gatewayid] = *wp;
        *wp = cp->withdrawinfos[Global_mp->gatewayid];
        printf("GOT <<<<<<<<<<<< publish_withdraw_info.%d coinid.%d %.8f crc %08x\n",gatewayid,wp->W.coinid,dstr(wp->W.amount),cp->withdrawinfos[gatewayid].rawtx.batchcrc);
    }
    return(sizeof(*wp));
}
// end of network funcs

int32_t ensure_wp(struct coin_info *cp,uint64_t amount,char *NXTaddr,char *redeemtxid)
{
    int32_t createdflag;
    struct withdraw_info *wp;
    if ( cp != 0 )
    {
        if ( is_limbo_redeem(cp->name,redeemtxid) != 0 )
            return(-1);
        wp = MTadd_hashtable(&createdflag,Global_mp->redeemtxids,redeemtxid);
        if ( createdflag != 0 )
        {
            wp->amount = amount;
            wp->coinid = conv_coinstr(cp->name);//coinid;
            strcpy(wp->NXTaddr,NXTaddr);
            printf("%s ensure redeem.%s %.8f -> NXT.%s\n",cp->name,redeemtxid,dstr(amount),NXTaddr);
        }
    } else if ( cp != 0 ) printf("Unexpected missing replicated_coininfo for coin.%s\n",cp->name);
    return(0);
}

char *calc_withdraw_addr(char *destaddr,char *NXTaddr,struct coin_info *cp,struct NXT_assettxid *tp,struct NXT_asset *ap)
{
    char pubkey[1024],withdrawaddr[1024];
    int64_t amount,minwithdraw;
    cJSON *obj,*argjson = 0;
   // struct coin_acct *acct;
    destaddr[0] = withdrawaddr[0] = 0;
    if ( tp->comment != 0 && tp->comment[0] != 0 && (argjson= cJSON_Parse(tp->comment)) != 0 )
    {
        obj = cJSON_GetObjectItem(argjson,"withdrawaddr");
        copy_cJSON(withdrawaddr,obj);
    }
    amount = tp->quantity * ap->mult;
    minwithdraw = cp->txfee * MIN_DEPOSIT_FACTOR;//get_min_withdraw(coinid);
    if ( amount <= minwithdraw )
    {
        printf("minimum withdrawal must be more than %.8f %s\n",dstr(minwithdraw),cp->name);
        if ( argjson != 0 )
            free_json(argjson);
        return(0);
    }
    else if ( tp->redeemtxid == 0 )
    {
        printf("no redeem txid %s %s\n",cp->name,cJSON_Print(argjson));
        if ( argjson != 0 )
            free_json(argjson);
        return(0);
    }
    if ( withdrawaddr[0] == 0 )
    {
        printf("no withdraw address for %.8f | ",dstr(amount));
        if ( argjson != 0 )
            free_json(argjson);
        return(0);
    }
    //printf("withdraw addr.(%s) lp.%p\n",withdrawaddr,lp);
    if ( cp != 0 && validate_coinaddr(pubkey,cp,withdrawaddr) < 0 )
    {
        printf("invalid address.(%s) for NXT.%s %.8f validate.%d\n",withdrawaddr,NXTaddr,dstr(amount),validate_coinaddr(pubkey,cp,withdrawaddr));
        if ( argjson != 0 )
            free_json(argjson);
        return(0);
    }
    if ( argjson != 0 )
        free_json(argjson);
    strcpy(destaddr,withdrawaddr);
    return((destaddr[0] != 0) ? destaddr : 0);
}

uint64_t add_pendingxfer(int32_t removeflag,uint64_t txid)
{
    static int numpending;
    static uint64_t *pendingxfers;
    int32_t nonz,i = 0;
    uint64_t pendingtxid = 0;
    nonz = 0;
    if ( numpending > 0 )
    {
        for (i=0; i<numpending; i++)
        {
            if ( removeflag == 0 )
            {
                if ( pendingxfers[i] == 0 )
                {
                    pendingxfers[i] = txid;
                    break;
                } else nonz++;
            }
            else if ( pendingxfers[i] == txid )
            {
                printf("PENDING.(%llu) removed\n",(long long)txid);
                pendingxfers[i] = 0;
                return(0);
            }
        }
    }
    if ( i == numpending && txid != 0 && removeflag == 0 )
    {
        pendingxfers = realloc(pendingxfers,sizeof(*pendingxfers) * (numpending+1));
        pendingxfers[numpending++] = txid;
    }
    if ( numpending > 0 )
    {
        for (i=0; i<numpending; i++)
        {
            if ( pendingtxid == 0 && pendingxfers[i] != 0 )
            {
                pendingtxid = pendingxfers[i];
                break;
            }
        }
    }
    return(pendingtxid);
}

void process_MGW_message(struct json_AM *ap,char *sender,char *receiver,char *txid,int32_t syncflag,char *coinstr)
{
    char NXTaddr[64];
    cJSON *argjson;
    struct multisig_addr *msig;
    expand_nxt64bits(NXTaddr,ap->H.nxt64bits);
    if ( (argjson = parse_json_AM(ap)) != 0 )
    {
        //printf("func.(%c) %s -> %s txid.(%s) JSON.(%s)\n",ap->funcid,sender,receiver,txid,ap->U.jsonstr);
        switch ( ap->funcid )
        {
            case GET_COINDEPOSIT_ADDRESS:
                // start address gen
                //update_coinacct_addresses(ap->H.nxt64bits,argjson,txid,-1);
                break;
            case BIND_DEPOSIT_ADDRESS:
                if ( (msig= decode_msigjson(0,argjson,sender)) != 0 )
                {
                    //printf("%s func.(%c) %s -> %s txid.(%s) JSON.(%s)\n",msig->coinstr,ap->funcid,sender,receiver,txid,ap->U.jsonstr);
                    if ( strcmp(msig->coinstr,coinstr) == 0 )
                    {
                        if ( update_msig_info(msig,syncflag) == 0 )
                            printf("%s func.(%c) %s -> %s txid.(%s) JSON.(%s)\n",msig->coinstr,ap->funcid,sender,receiver,txid,ap->U.jsonstr);
                    }
                    free(msig);
                } //else printf("WARNING: sender.%s == NXTaddr.%s\n",sender,NXTaddr);
                break;
            case DEPOSIT_CONFIRMED:
                // need to mark cointxid with AMtxid to prevent confirmation process generating AM each time
                /*if ( is_gateway_addr(sender) != 0 && (coinid= decode_depositconfirmed_json(argjson,txid)) >= 0 )
                 {
                 printf("deposit confirmed for coinid.%d %s\n",coinid,coinid_str(coinid));
                 }*/
                break;
            case MONEY_SENT:
                //if ( is_gateway_addr(sender) != 0 )
                //    update_money_sent(argjson,txid,height);
                break;
            default: printf("funcid.(%c) not handled\n",ap->funcid);
        }
        if ( argjson != 0 )
            free_json(argjson);
    }// else printf("can't JSON parse (%s)\n",ap->U.jsonstr);
}

uint64_t process_NXTtransaction(char *sender,char *receiver,cJSON *item,char *refNXTaddr,char *assetid,int32_t syncflag,struct coin_info *cp)
{
    int32_t conv_coinstr(char *);
    char AMstr[4096],txid[1024],comment[1024],*assetidstr,*commentstr = 0;
    cJSON *senderobj,*attachment,*message,*assetjson,*commentobj,*cointxidobj;
    char cointxid[128];
    unsigned char buf[4096];
    struct NXT_AMhdr *hdr;
    struct NXT_asset *ap = 0;
    struct NXT_assettxid *tp;
    uint64_t retbits = 0;
    int32_t height,timestamp=0,coinid,dir = 0;
    int64_t type,subtype,n,assetoshis = 0;
    assetid[0] = 0;
    if ( item != 0 )
    {
        hdr = 0; assetidstr = 0;
        sender[0] = receiver[0] = 0;
        copy_cJSON(txid,cJSON_GetObjectItem(item,"transaction"));
        type = get_cJSON_int(item,"type");
        subtype = get_cJSON_int(item,"subtype");
        timestamp = (int32_t)get_cJSON_int(item,"blockTimestamp");
        height = (int32_t)get_cJSON_int(item,"height");
        senderobj = cJSON_GetObjectItem(item,"sender");
        if ( senderobj == 0 )
            senderobj = cJSON_GetObjectItem(item,"accountId");
        else if ( senderobj == 0 )
            senderobj = cJSON_GetObjectItem(item,"account");
        copy_cJSON(sender,senderobj);
        copy_cJSON(receiver,cJSON_GetObjectItem(item,"recipient"));
        attachment = cJSON_GetObjectItem(item,"attachment");
        if ( attachment != 0 )
        {
            message = cJSON_GetObjectItem(attachment,"message");
            assetjson = cJSON_GetObjectItem(attachment,"asset");
            if ( message != 0 && type == 1 )
            {
                copy_cJSON(AMstr,message);
                //printf("AM message.(%s).%ld\n",AMstr,strlen(AMstr));
                n = strlen(AMstr);
                if ( is_hexstr(AMstr) != 0 )
                {
                    if ( (n&1) != 0 || n > 2000 )
                        printf("warning: odd message len?? %ld\n",(long)n);
                    memset(buf,0,sizeof(buf));
                    decode_hex((void *)buf,(int32_t)(n>>1),AMstr);
                    hdr = (struct NXT_AMhdr *)buf;
                    process_MGW_message((void *)hdr,sender,receiver,txid,syncflag,cp->name);
                }
            }
            else if ( assetjson != 0 && type == 2 && subtype <= 1 )
            {
                commentobj = cJSON_GetObjectItem(attachment,"comment");
                if ( commentobj == 0 )
                    commentobj = message;
                copy_cJSON(comment,commentobj);
                if ( comment[0] != 0 )
                    commentstr = clonestr(replace_backslashquotes(comment));
                tp = add_NXT_assettxid(&ap,assetid,assetjson,txid,timestamp);
                if ( tp != 0 )
                {
                    if ( tp->comment != 0 )
                        free(tp->comment);
                    tp->comment = commentstr;
                    tp->timestamp = timestamp;
                    if ( type == 2 )
                    {
                        tp->quantity = get_cJSON_int(attachment,"quantityQNT");
                        assetoshis = tp->quantity;
                        switch ( subtype )
                        {
                            case 0:
                                break;
                            case 1:
                                tp->senderbits = calc_nxt64bits(sender);
                                tp->receiverbits = calc_nxt64bits(receiver);
                                if ( ap != 0 )
                                {
                                    coinid = conv_coinstr(ap->name);
                                    commentobj = 0;
                                    if ( ap->mult != 0 )
                                        assetoshis *= ap->mult;
                                    //printf("case1 sender.(%s) receiver.(%s) comment.%p cmp.%d\n",sender,receiver,tp->comment,strcmp(receiver,refNXTaddr)==0);
                                    if ( tp->comment != 0 && (commentobj= cJSON_Parse(tp->comment)) != 0 )
                                    {
                                        cointxidobj = cJSON_GetObjectItem(commentobj,"cointxid");
                                        if ( cointxidobj != 0 )
                                        {
                                            copy_cJSON(cointxid,cointxidobj);
                                            printf("got comment.(%s) cointxidstr.(%s)\n",tp->comment,cointxid);
                                            if ( cointxid[0] != 0 )
                                                tp->cointxid = clonestr(cointxid);
                                        } else cointxid[0] = 0;
                                        free_json(commentobj);
                                    }
                                    if ( coinid >= 0 && is_limbo_redeem(ap->name,txid) == 0 )
                                    {
                                        if ( strcmp(receiver,refNXTaddr) == 0 )
                                        {
                                            if ( Debuglevel > 2 )
                                                printf("%s got comment.(%s) gotredeem.(%s) coinid.%d %.8f\n",ap->name,tp->comment,cointxid,coinid,dstr(tp->quantity * ap->mult));
                                            tp->redeemtxid = calc_nxt64bits(txid);
                                            //printf("protocol redeem.(%s)\n",txid);
                                            dir = -1;
                                            if ( tp->comment != 0 )
                                                tp->completed = MGW_PENDING_WITHDRAW;
                                            ensure_wp(get_coin_info(ap->name),tp->quantity * ap->mult,sender,txid);
                                        }
                                        else if ( strcmp(sender,refNXTaddr) == 0 )
                                            dir = 1;
                                    }
                                }
                                break;
                            case 2:
                            case 3: // bids and asks, no indication they are filled at this point, so nothing to do
                                break;
                        }
                    }
                    add_pendingxfer(1,tp->txidbits);
                    retbits = tp->txidbits;
                }
            }
        }
    }
    else printf("unexpected error iterating timestamp.(%d) txid.(%s)\n",timestamp,txid);
    return(retbits);//assetoshis * dir);
}

int32_t init_NXT_transactions(int32_t txtype,char *refNXTaddr,struct coin_info *cp)
{
    char sender[1024],receiver[1024],assetid[1024],cmd[1024],*jsonstr;
    int32_t createdflag,coinid,i,n = 0;
    uint32_t timestamp;
    struct NXT_acct *np;
    cJSON *item,*json,*array;
    if ( refNXTaddr == 0 )
    {
        printf("illegal refNXT.(%s)\n",refNXTaddr);
        return(0);
    }
    sprintf(cmd,"%s=getAccountTransactions&account=%s&type=%d",_NXTSERVER,refNXTaddr,txtype);
    coinid = conv_coinstr(cp->name);
    np = get_NXTacct(&createdflag,Global_mp,refNXTaddr);
    if ( coinid > 0 && np->timestamps[coinid] != 0 )
        sprintf(cmd + strlen(cmd),"&timestamp=%d",cp->timestamps[coinid]);
    if ( Debuglevel > 2 )
        printf("init_NXT_transactions.(%s) for (%s) cmd.(%s)\n",refNXTaddr,cp->name,cmd);
    if ( (jsonstr= issue_NXTPOST(0,cmd)) != 0 )
    {
        //printf("(%s)\n",jsonstr);
        if ( (json= cJSON_Parse(jsonstr)) != 0 )
        {
            if ( (array= cJSON_GetObjectItem(json,"transactions")) != 0 && is_cJSON_Array(array) != 0 && (n= cJSON_GetArraySize(array)) > 0 )
            {
                for (i=0; i<n; i++)
                {
                    if ( Debuglevel > 2 )
                        fprintf(stderr,"%d/%d ",i,n);
                    item = cJSON_GetArrayItem(array,i);
                    process_NXTtransaction(sender,receiver,item,refNXTaddr,assetid,0,cp);
                    timestamp = (int32_t)get_cJSON_int(item,"blockTimestamp");
                    if ( timestamp > np->timestamps[coinid] )
                        np->timestamps[coinid] = timestamp;
                }
            }
            free_json(json);
        }
        free(jsonstr);
    } else printf("error with init_NXT_transactions.(%s)\n",cmd);
    return(n);
}

int32_t ready_to_xferassets(uint64_t *txidp)
{
    // if fresh reboot, need to wait the xfer max duration + 1 block before running this
    static int32_t firsttime,firstNXTblock;
    *txidp = 0;
    printf("(%d %d) lag.%ld %d\n",firsttime,firstNXTblock,time(NULL)-firsttime,get_NXTblock(0)-firstNXTblock);
    if ( firsttime == 0 )
        firsttime = (uint32_t)time(NULL);
    if ( firstNXTblock <= 0 )
        firstNXTblock = get_NXTblock(0);
    if ( time(NULL) < (firsttime + DEPOSIT_XFER_DURATION*60) )
        return(0);
    if ( firstNXTblock <= 0 || get_NXTblock(0) < (firstNXTblock + 3) )
        return(0);
    if ( (*txidp= add_pendingxfer(0,0)) != 0 )
    {
        printf("waiting for pendingxfer\n");
        return(0);
    }
    return(1);
}

uint64_t process_msigdeposits(cJSON **transferjsonp,int32_t forceflag,struct coin_info *cp,struct address_entry *entry,uint64_t nxt64bits,struct NXT_asset *ap,char *msigaddr)
{
    char txidstr[1024],coinaddr[1024],script[4096],coinaddr_v0[1024],script_v0[4096],comment[4096],NXTaddr[64],numstr[64];
    struct NXT_assettxid *tp;
    uint64_t depositid,value,total = 0;
    int32_t j,numvouts;
    cJSON *pair;
    for (j=0; j<ap->num; j++)
    {
        tp = ap->txids[j];
        //printf("%d of %d: process.(%s) isinternal.%d %llu (%llu -> %llu)\n",j,ap->num,msigaddr,entry->isinternal,(long long)nxt64bits,(long long)tp->senderbits,(long long)tp->receiverbits);
        if ( tp->receiverbits == nxt64bits && tp->coinblocknum == entry->blocknum && tp->cointxind == entry->txind && tp->coinv == entry->v )
            break;
    }
    if ( j == ap->num )
    {
        value = get_txoutstr(&numvouts,txidstr,coinaddr,script,cp,entry->blocknum,entry->txind,entry->v);
        if ( strcmp("31dcbc5b7cfd7fc8f2c1cedf65f38ec166b657cc9eb15e7d1292986eada35ea9",txidstr) == 0 ) // due to uncommented tx
            return(0);
        if ( entry->v == numvouts-1 )
        {
            get_txoutstr(0,txidstr,coinaddr_v0,script_v0,cp,entry->blocknum,entry->txind,0);
            if ( strcmp(coinaddr_v0,cp->marker) == 0 )
                return(0);
        }
        if ( strcmp(msigaddr,coinaddr) == 0 && txidstr[0] != 0 && value >= (cp->NXTfee_equiv * MIN_DEPOSIT_FACTOR) )
        {
            for (j=0; j<ap->num; j++)
            {
                tp = ap->txids[j];
                if ( tp->receiverbits == nxt64bits && tp->cointxid != 0 && strcmp(tp->cointxid,txidstr) == 0 )
                {
                    if ( Debuglevel > 0 )
                        printf("%llu set cointxid.(%s) <-> (%u %d %d)\n",(long long)nxt64bits,txidstr,entry->blocknum,entry->txind,entry->v);
                    tp->cointxind = entry->txind;
                    tp->coinv = entry->v;
                    tp->coinblocknum = entry->blocknum;
                    break;
                }
            }
            if ( j == ap->num )
            {
                //printf("UNPAID cointxid.(%s) <-> (%u %d %d)\n",txidstr,entry->blocknum,entry->txind,entry->v);
                sprintf(comment,"{\"coinaddr\":\"%s\",\"cointxid\":\"%s\",\"coinblocknum\":%u,\"cointxind\":%u,\"coinv\":%u}",coinaddr,txidstr,entry->blocknum,entry->txind,entry->v);
                printf(">>>>>>>>>>>>>> Need to transfer %.8f %ld assetoshis | %s to %llu for (%s) %s\n",dstr(value),(long)(value/ap->mult),cp->name,(long long)nxt64bits,txidstr,comment);
                total += value;
                if ( forceflag > 0 )// || ready_to_xferassets() > 0 )
                {
                    expand_nxt64bits(NXTaddr,nxt64bits);
                    depositid = issue_transferAsset(0,0,cp->srvNXTACCTSECRET,NXTaddr,cp->assetid,value/ap->mult,MIN_NQTFEE,DEPOSIT_XFER_DURATION,comment);
                    add_pendingxfer(0,depositid);
                    if ( transferjsonp != 0 )
                    {
                        if ( *transferjsonp == 0 )
                            *transferjsonp = cJSON_CreateArray();
                        pair = cJSON_Parse(comment);
                        sprintf(numstr,"\"%llu\"",(long long)depositid);
                        cJSON_AddItemToObject(pair,"depositid",cJSON_CreateString(numstr));
                        cJSON_AddItemToArray(*transferjsonp,pair);
                    }
                }
            }
        }
    }
    return(total);
}

struct coin_txidmap *get_txid(struct coin_info *cp,char *txidstr,uint32_t blocknum,int32_t txind,int32_t v)
{
    char buf[1024],coinaddr[1024],script[4096],checktxidstr[1024];
    int32_t createdflag;
    struct coin_txidmap *tp;
    sprintf(buf,"%s_%s_%d",txidstr,cp->name,v);
    tp = MTadd_hashtable(&createdflag,Global_mp->coin_txidmap,buf);
    if ( createdflag != 0 )
    {
        if ( blocknum == 0xffffffff || txind < 0 )
        {
            blocknum = get_txidind(&txind,cp,txidstr,v);
            if ( txind >= 0 && blocknum < 0xffffffff )
            {
                get_txoutstr(0,checktxidstr,coinaddr,script,cp,blocknum,txind,v);
                if ( strcmp(checktxidstr,txidstr) != 0 )
                    printf("checktxid.(%s) != (%s)???\n",checktxidstr,txidstr);
                else printf("txid.(%s) (%d %d %d) verified\n",txidstr,blocknum,txind,v);
            }
        }
        tp->blocknum = blocknum;
        tp->txind = txind;
        tp->v = v;
    }
    return(tp);
}

struct coin_txidind *get_cointp(struct coin_info *cp,struct address_entry *entry)
{
    char indstr[32],script[MAX_JSON_FIELD],origtxidstr[256];
    struct coin_txidind *cointp;
    struct coin_txidmap *tp;
    uint64_t ind;
    int32_t createdflag,spentflag;
    uint32_t blocknum;
    uint16_t txind,v;
    if ( entry->vinflag != 0 )
    {
        v = get_txinstr(origtxidstr,cp,entry->blocknum,entry->txind,entry->v);
        tp = get_txid(cp,origtxidstr,0xffffffff,-1,v);
        blocknum = tp->blocknum;
        txind = tp->txind;
        if ( v != tp->v )
            fprintf(stderr,"error (%d != %d)\n",v,tp->v);
        if ( Debuglevel > 2 )
            printf("get_cointp.(%016llx) spent.(%d %d %d) -> (%s).%d (%d %d %d)\n",*(long long *)entry,entry->blocknum,entry->txind,entry->v,origtxidstr,v,blocknum,txind,v);
        spentflag = 1;
    }
    else
    {
        blocknum = entry->blocknum;
        txind = entry->txind;
        v = entry->v;
        spentflag = 0;
    }
    ind = ((uint64_t)blocknum << 32) | ((uint64_t)txind << 16) | v;
    strcpy(indstr,cp->name);
    expand_nxt64bits(indstr+strlen(indstr),ind);
    cointp = MTadd_hashtable(&createdflag,Global_mp->coin_txidinds,indstr);
    if ( createdflag != 0 )
    {
        cointp->entry = *entry;
        cointp->value = get_txoutstr(&cointp->numvouts,cointp->txid,cointp->coinaddr,script,cp,blocknum,txind,v);
        cointp->script = clonestr(script);
        if ( entry->vinflag == 0 )
            get_txid(cp,cointp->txid,blocknum,txind,v);
    }
    if ( spentflag != 0 )
        cointp->entry.spent = 1;
    if ( cointp->entry.spent != 0 && cointp->script != 0 )
        free(cointp->script), cointp->script = 0;
    return(cointp);
}

uint64_t process_msigaddr(int32_t *numunspentp,uint64_t *unspentp,cJSON **transferjsonp,int32_t forceflag,struct NXT_asset *ap,char *refassetid,char *NXTaddr,struct coin_info *cp,char *msigaddr)
{
    struct address_entry *entries,*entry;
    int32_t i,n;
    uint32_t createtime;
    struct coin_txidind *cointp;
    uint64_t nxt64bits,unspent,amount = 0;
    if ( ap->mult == 0 )
    {
        printf("ap->mult is ZERO for %s?\n",refassetid);
        return(0);
    }
    nxt64bits = calc_nxt64bits(NXTaddr);
    if ( (entries= get_address_entries(&n,cp->name,msigaddr)) != 0 )
    {
        if ( Debuglevel > 2 )
            printf(">>>>>>>>>>>>>>>> %d address entries for (%s)\n",n,msigaddr);
        for (i=0; i<n; i++)
        {
            entry = &entries[i];
            if ( entry->vinflag == 0 )
                amount += process_msigdeposits(transferjsonp,forceflag,cp,entry,nxt64bits,ap,msigaddr);
            if ( Debuglevel > 2 )
                printf("process_msigaddr.(%s) %d of %d: vin.%d internal.%d spent.%d (%d %d %d)\n",msigaddr,i,n,entry->vinflag,entry->isinternal,entry->spent,entry->blocknum,entry->txind,entry->v);
            get_cointp(cp,entry);
        }
        for (i=0; i<n; i++)
        {
            entry = &entries[i];
            cointp = get_cointp(cp,entry);
            if ( cointp != 0 && cointp->entry.spent == 0 )
            {
                unspent = cointp->value;
                /*if ( (unspent= check_txout(&createtime,cp,cp->minconfirms,0,cointp->txid,cointp->entry.v,0)) == 0 )
                    cointp->entry.spent = 1;
                else if ( unspent != cointp->value )
                    printf("ERROR: %.8f != %.8f | %s %s.%d\n",dstr(unspent),dstr(cointp->value),cp->name,cointp->txid,cointp->entry.v);
                else*/
                {
                    cointp->unspent = unspent;
                    printf("%s | %.8f\n",cointp->txid,dstr(cointp->unspent));
                    (*numunspentp)++;
                    (*unspentp) += unspent;
                    update_unspent_funds(cp,cointp,0);
                }
            }
        }
        free(entries);
    }
    return(amount);
}

int32_t valid_msig(struct multisig_addr *msig,char *coinstr,char *specialNXT,char *gateways[],char *ipaddrs[],int32_t M,int32_t N)
{
    int32_t i,match = 0;
    char NXTaddr[64],gatewayNXTaddr[64],ipaddr[64];
    //printf("%s %s M.%d N.%d %llu vs %s (%s %s %s)\n",msig->coinstr,coinstr,msig->m,msig->n,(long long)msig->sender,specialNXT,gateways[0],gateways[1],gateways[2]);
    if ( strcmp(msig->coinstr,coinstr) == 0 && msig->m == M && msig->n == N )
    {
        expand_nxt64bits(NXTaddr,msig->sender);
        if ( strcmp(NXTaddr,specialNXT) == 0 )
            match++;
        else
        {
            for (i=0; i<N; i++)
                if ( strcmp(NXTaddr,gateways[i]) == 0 )
                    match++;
        }
return(match);
        if ( match > 0 )
        {
            printf("match.%d check for sender.(%s) vs special %s %s %s %s\n",match,NXTaddr,specialNXT,gateways[0],gateways[1],gateways[2]);
            for (i=0; i<N; i++)
            {
                expand_nxt64bits(gatewayNXTaddr,msig->pubkeys[i].nxt64bits);
                if ( strcmp(gateways[i],gatewayNXTaddr) != 0 )
                {
                    printf("(%s != %s) ",gateways[i],gatewayNXTaddr);
                    break;
                }
            }
            printf("i.%d\n",i);
            if ( i == N )
                return(1);
            for (i=0; i<N; i++)
            {
                expand_ipbits(ipaddr,msig->pubkeys[i].ipbits);
                printf("(%s) ",ipaddr);
                if ( strcmp(ipaddrs[i],ipaddr) != 0 )
                    break;
            }
            printf("j.%d\n",i);
            if ( i == N )
                return(1);
        }
    }
    return(0);
}

char *MGWdeposits(char *specialNXT,int32_t rescan,int32_t transferassets,char *coin,char *assetstr,char *NXT0,char *NXT1,char *NXT2,char *ip0,char *ip1,char *ip2)
{
    static int32_t firsttimestamp;
    int32_t numgateways = 3;
    char retbuf[1024],*gateways[3],*ipaddrs[3],*retstr,*transferstr = 0;
    struct coin_info *cp,*btcdcp;
    cJSON *json,*transferjson = 0;
    struct unspent_info *up;
    uint64_t pendingtxid,circulation,unspent,total = 0;
    int32_t i,n,nonz,numunspent,createdflag,readyflag;
    struct NXT_asset *ap;
    struct storage_header **msigs;
    struct multisig_addr *msig;
    ap = get_NXTasset(&createdflag,Global_mp,assetstr);
    cp = conv_assetid(assetstr);
    if ( cp == 0 )
    {
        sprintf(retbuf,"{\"error\":\"dont have coin_info for (%s)\"}",assetstr);
        return(clonestr(retbuf));
    }
    up = &cp->unspent;
    if ( firsttimestamp == 0 )
        get_NXTblock(&firsttimestamp);
    gateways[0] = NXT0, gateways[1] = NXT1, gateways[2] = NXT2;
    ipaddrs[0] = ip0, ipaddrs[1] = ip1, ipaddrs[2] = ip2;
    for (i=0; i<numgateways; i++)
    {
        if ( gateways[i] == 0 )
            gateways[i] = "";
        if ( ipaddrs[i] == 0 )
            strcpy(ipaddrs[i],Server_names[i]);
    }
    ready_to_xferassets(&pendingtxid);
    if ( rescan != 0 ) // side effect updates MULTISIG_DATA from AM
    {
        if ( (btcdcp= get_coin_info("BTCD")) != 0 )
        {
            init_NXT_transactions(1,btcdcp->srvNXTADDR,cp);
            init_NXT_transactions(1,btcdcp->privateNXTADDR,cp);
        }
        init_NXT_transactions(1,specialNXT,cp);
        for (i=0; i<numgateways; i++)
            init_NXT_transactions(1,gateways[i],cp);
        update_msig_info(0,1);
    }
    circulation = unspent = 0;
    if ( ready_to_xferassets(&pendingtxid) <= 0 && pendingtxid != 0 )
    {
        char txidstr[64],sender[64],receiver[64];
        uint64_t val;
        expand_nxt64bits(txidstr,pendingtxid);
        if ( (retstr= issue_getTransaction(0,txidstr)) != 0 )
        {
            if ( (json= cJSON_Parse(retstr)) != 0 )
            {
                if ( (val= process_NXTtransaction(sender,receiver,json,specialNXT,assetstr,1,cp)) != 0 )
                    printf(">>>>>>>>>>>>>> processed %llu vs %llu\n",(long long)val,(long long)pendingtxid);
                free_json(json);
            }
            free(retstr);
        }
    }
    if ( (msigs= copy_all_DBentries(&n,MULTISIG_DATA)) != 0 )
    {
        readyflag = ready_to_xferassets(&pendingtxid);
        if ( Debuglevel > 1 )
            printf("got n.%d msigs readyflag.%d\n",n,readyflag);
        update_unspent_funds(cp,0,n*1000);
        nonz = numunspent = 0;
        for (i=0; i<n; i++)
        {
            //fprintf(stderr,"(%d %p) ",i,msigs[i]);
            if ( (msig= (struct multisig_addr *)msigs[i]) != 0 )
            {
                if ( valid_msig(msig,coin,specialNXT,gateways,ipaddrs,numgateways-1,numgateways) != 0 )
                {
                    if ( Debuglevel > 2 )
                        printf("MULTISIG: %s: %d of %d %s %s\n",cp->name,i,n,msig->coinstr,msig->multisigaddr);
                    if ( strcmp(msig->coinstr,cp->name) == 0 )
                    {
                        init_NXT_transactions(2,msig->NXTaddr,cp);
                        if ( readyflag > 0 )
                        {
                            int32_t tmp;
                            tmp = numunspent;
                            total += (process_msigaddr(&numunspent,&unspent,&transferjson,transferassets,ap,assetstr,msig->NXTaddr,cp,msig->multisigaddr) > 0);
                            if ( numunspent > tmp )
                                nonz++;
                        }
                    }
                }
                free(msig);
            } 
        }
        if ( up->num > 1 )
            sort_vps(up->vps,up->num);
        printf("max %.8f min %.8f median %.8f |unspent %.8f numunspent.%d in nonz.%d accts\n",dstr(up->maxavail),dstr(up->minavail),dstr((up->maxavail+up->minavail)/2),dstr(up->unspent),numunspent,nonz);
        free(msigs);
    }
    if ( transferjson != 0 )
    {
        transferstr = cJSON_Print(transferjson);
        free_json(transferjson);
        stripwhite_ns(transferstr,strlen(transferstr));
        retstr = malloc(strlen(transferstr) + 4096);
    } else retstr = malloc(4096);
    sprintf(retstr,"{\"circulation\":\"%.8f\",\"unspent\":\"%.8f\",\"pendingdeposits\":\"%.8f\",\"transfers\":%s}\n",dstr(circulation),dstr(unspent),dstr(total),transferstr!=0?transferstr:"[]");
    if ( transferstr != 0 )
        free(transferstr);
    return(retstr);
}
#endif


