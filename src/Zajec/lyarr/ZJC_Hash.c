#include "ZJC_Hash.h"
#include "../lyutils/ZJC_Evil.h"

#include <stdio.h>

//  - --- - --- - --- - --- -

Hash* ZJC_build_hash(uint size,
                     uint max_value)            {

    Hash* hash      = NULL;

    WARD_EVIL_MALLOC                            (hash,        Hash, sizeof(Hash), 1                                     );
    WARD_EVIL_MALLOC                            (hash->table, LND*, sizeof(LND*), size                                  );

    hash->max_value = max_value;
    hash->size      = size;
    hash->xfac      = 1;

    if(max_value > size)                        { hash->xfac = max_value / size;                                        }
    return hash;                                                                                                        }

void  ZJC_del_hash(Hash* hash)                  {

    for(uint i = 0; i < hash->size; i++)        { ZJC_del_linkList(hash->table[i]);                                     }

    WARD_EVIL_MFREE(hash->table);
    WARD_EVIL_MFREE(hash       );                                                                                       }

//  - --- - --- - --- - --- -

uint ZJC_hashit(Hash* hash, uint id)            { return (id + (id/hash->xfac)) % hash->size;                           }

void ZJC_push_hash(Hash* hash,
                   uint id,
                   uint data   )                { uint loc    = ZJC_hashit(hash, id); MEAN_NZID_WARNING("ZJC_Hash", id);

    if(hash->table[loc] == NULL)                { uint  nid  [1  ] = { id }; uint ndata[1] = { data };
                                                  hash->table[loc] = ZJC_build_linkList( nid, ndata, 1 );               }

    else                                        { ZJC_push_linkList(hash->table[loc], id, data);                        }
                                                                                                                        }

uint ZJC_pop_hash(Hash* hash, uint id)          { uint loc         = ZJC_hashit       (hash,              id );

    uint      data   = 0;                         LND* tmp         = ZJC_find_linkList( hash->table[loc], id );
    if(tmp) { data   = tmp->data;                 hash->table[loc] = ZJC_pop_linkList ( hash->table[loc], 0  );         }

    return    data;                                                                                                     }

uint ZJC_find_hash(Hash* hash, uint id)         { uint loc = ZJC_hashit(hash, id);

    LND* tmp = ZJC_find_linkList                ( hash->table[loc], id                                                  );
    if(tmp)                                     { return tmp->data;                                                     }

    return 0;                                                                                                           }

//  - --- - --- - --- - --- -
