/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include "misc.h"
#include "locks.h"

ap_status_t ap_create_pool(ap_pool_t **newcont, ap_pool_t *cont)
{
    ap_pool_t *new;

    if (cont) {
        new = ap_make_sub_pool(cont, cont->apr_abort);
    }
    else {
        new = ap_make_sub_pool(NULL, NULL);
    }
        
    if (new == NULL) {
        return APR_ENOPOOL;
    }   

    new->prog_data = NULL;
    new->apr_abort = NULL;
 
    *newcont = new;
    return APR_SUCCESS;
}

ap_status_t ap_set_userdata(void *data, char *key,
                            ap_status_t (*cleanup) (void *),
                            ap_pool_t *cont)
{
    datastruct *dptr = NULL, *dptr2 = NULL;
    if (cont) { 
        dptr = cont->prog_data;
        while (dptr) {
            if (!strcmp(dptr->key, key))
                break;
            dptr2 = dptr;
            dptr = dptr->next;
        }
        if (dptr == NULL) {
            dptr = ap_pcalloc(cont, sizeof(datastruct));
            dptr->next = dptr->prev = NULL;
            dptr->key = ap_pstrdup(cont, key);
            if (dptr2) {
                dptr2->next = dptr;
                dptr->prev = dptr2;
            }
            else {
                cont->prog_data = dptr;
            }
        }
        dptr->data = data;
        ap_register_cleanup(cont, dptr->data, cleanup, cleanup);
        return APR_SUCCESS;
    }
    return APR_ENOPOOL;
}

ap_status_t ap_get_userdata(void **data, char *key, ap_pool_t *cont)
{
    datastruct *dptr = NULL;
    if (cont) { 
        dptr = cont->prog_data;
        while (dptr) {
            if (!strcmp(dptr->key, key)) {
                break;
            }
            dptr = dptr->next;
        }
        if (dptr) {
            (*data) = dptr->data;
        }
        else {
            (*data) = NULL;
        }
        return APR_SUCCESS;
    }
    return APR_ENOPOOL;
}

ap_status_t ap_initialize(void)
{
    ap_status_t status;
#if !defined(BEOS) && !defined(OS2) && !defined(WIN32)
    ap_unix_setup_lock();
#elif defined WIN32
    int iVersionRequested;
    WSADATA wsaData;
    int err;

    iVersionRequested = MAKEWORD(WSAHighByte, WSALowByte);
    err = WSAStartup((WORD) iVersionRequested, &wsaData);
    if (err) {
        return err;
    }
    if (LOBYTE(wsaData.wVersion) != WSAHighByte ||
        HIBYTE(wsaData.wVersion) != WSALowByte) {
        WSACleanup();
        return APR_EEXIST;
    }
#endif
    status = ap_init_alloc();
    return status;
}

void ap_terminate(void)
{
    ap_term_alloc();
}

ap_status_t ap_set_abort(int (*apr_abort)(int retcode), ap_pool_t *cont)
{
    if (cont == NULL) {
        return APR_ENOPOOL;
    }
    else {
        cont->apr_abort = apr_abort;
        return APR_SUCCESS;
    }
}
 
