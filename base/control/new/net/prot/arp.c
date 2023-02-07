

// arp.c

#include <kernel.h>

unsigned char __arp_broadcast_mac[6] = { 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF 
};
unsigned char __arp_gramado_default_ipv4[4] = { 
    192, 168, 1, 12 
};
unsigned char __arp_target_default_ipv4[4]  = { 
    192, 168, 1, 6 
};
unsigned char __arp_gateway_default_ipv4[4] = { 
    192, 168, 1, 1 
};

// handle arp package
// Called by all the embedded nic device drivers.
// IN:
// buffer = The address after the ethernet header.
void 
network_handle_arp( 
    const unsigned char *buffer, 
    ssize_t size )
{
// handler
// 0x0806

    struct ether_arp *ar;
    ar = (struct ether_arp *) buffer;

    if ( (void*) ar == NULL ){
        printf("network_handle_arp: ar\n");
        goto fail;
    }

/*
// Show data.
// Bytes: Net-style.
// Hardware type (HTYPE)   (00 01) = Ethernet.
    printf("Hardware type: {%x}\n",ar->type);
// Protocol type (PTYPE)   (08 00) = ipv4.
// In the case of Ethernet, a 0x0806 EtherType value 
// is used to identify ARP frames.
    printf("Protocol type: {%x}\n",ar->proto);
    printf("Hardware address lenght: {%x}\n",ar->hlen);
    printf("Protocol address lenght: {%x}\n",ar->plen);
    printf("ARP operation {%x}\n",ar->op);
    // continua: macs e ips ... 
*/

// Operation
    uint16_t op = (uint16_t) FromNetByteOrder16(ar->op);
    if (op==ARP_OPC_REQUEST){
        printf("ARP: This is a REQUEST\n");
        // Sending a reply, only for linux
            if ( ar->arp_spa[3] == 6 ){
                   // #bugbug
                   // O reply esta usando o mac broadcast.
                   // tem que ser o mac de quem fez o request.
                   network_send_arp_reply();
            }

    } else if (op==ARP_OPC_REPLY){
        printf("ARP: REPLY to %d.%d.%d.%d\n",
            ar->arp_tpa[0], ar->arp_tpa[1], ar->arp_tpa[2], ar->arp_tpa[3] );
    };

    refresh_screen();
    return;

fail:
    printf("network_handle_arp: Fail\n");
    refresh_screen();
    return;
}


// Ethernet + arp
void 
network_send_arp( 
    int op,   //operation
    uint8_t source_ip[4], 
    uint8_t target_ip[4], 
    uint8_t target_mac[6] )
{
// Send ARP.

    struct ether_header *eh;
    struct ether_arp *h;
    register int i=0;

    // #debug
    printf("network_send_arp:\n");
    refresh_screen();

// The structure for the Intel NIC device.
    if ( (void*) currentNIC == NULL ){
        printf ("network_send_arp: currentNIC fail\n");
        goto fail;
    }

// A given IP number for Gramado.
// Saving it into the NIC structure.
// 192.168.1.12
    currentNIC->ip_address[0] = source_ip[0];  //192;
    currentNIC->ip_address[1] = source_ip[1];  //168;
    currentNIC->ip_address[2] = source_ip[2];  //1;
    currentNIC->ip_address[3] = source_ip[3];  //12;

//==============================================
// # ethernet header #
    eh = (void *) kmalloc( sizeof(struct ether_header) );
    if ( (void *) eh == NULL){
        printf ("network_send_arp: eh struct fail\n");
        goto fail;
    }
// MAC
// Save the source and the destination mac into the ethernet header.
// The destination mac came via argument.
    for ( i=0; i<ETH_ALEN; i++){
        eh->mac_src[i] = (uint8_t) currentNIC->mac_address[i];
        eh->mac_dst[i] = (uint8_t) target_mac[i];
    };
// TYPE
    eh->type = (uint16_t) ToNetByteOrder16(ETH_TYPE_ARP);

//==============================================
// # arp header #
    h = (void *) kmalloc ( sizeof(struct  ether_arp) );
    if ( (void *) h == NULL ){
        printf ("network_send_arp: struct h fail");
        goto fail;
    }

//
// Header
// see: arp.h
//

// Hardware type (HTYPE)   (00 01) = Ethernet.
    h->type = (uint16_t) 0x0100;
// Protocol type (PTYPE)   (08 00) = ipv4.
// In the case of Ethernet, a 0x0806 EtherType value 
// is used to identify ARP frames.
    h->proto = (uint16_t) 0x0008;
// Hardware address length (MAC)
    h->hlen = (uint8_t) ETH_ALEN;
// Protocol address length (IP)
    h->plen = (uint8_t) 4;
// Operation (OPER)
// We only have two valid operation codes.
    if (op != ARP_OPC_REQUEST && op != ARP_OPC_REPLY){
        panic("network_send_arp: Invalid operation code\n");
    }
    h->op = (uint16_t) ToNetByteOrder16(op);

//
// Addresses
//

// MAC addresses
// Hardware address
    for ( i=0; i<ETH_ALEN; i++ ){
        h->arp_sha[i] = (uint8_t) currentNIC->mac_address[i];  //my MAC
        h->arp_tha[i] = (uint8_t) target_mac[i]; 
    };
// IP addresses
// Protocol address
    for ( i=0; i<4; i++ ){
        h->arp_spa[i] = (uint8_t) source_ip[i]; 
        h->arp_tpa[i] = (uint8_t) target_ip[i]; 
    };

//==================================
// #debug
// show arp header.

/*
    printf("\n\n");
    printf("[arp]\n\n");
    printf("type={%x} proto={%x} hlen={%d} plen={%d} op={%x} \n", 
        h->type ,h->proto ,h->hlen ,h->plen ,h->op);
    printf("\n sender: mac ");
    for( i=0; i<6; i++){ printf("%x ",h->arp_sha[i]); }
    printf("\n sender: ip ");
    for( i=0; i<4; i++){ printf("%d ",h->arp_spa[i]); }
    printf("\n target: mac ");
    for( i=0; i<6; i++){ printf("%x ",h->arp_tha[i]); }
    printf("\n target: ip ");
    for( i=0; i<4; i++){ printf("%d ",h->arp_tpa[i]); }
    refresh_screen();
    while(1){}
*/
//==================================

//
// Buffer
//

// Set up the pointers for the copy.

// Offset do endereço do buffer.
// Estamos pegando o offset que nos leva ao endereço do buffer.
// Pegamos esse offset na estrutura do controlador nic intel.
// see: nicintel.h

// pega o tail antigo.
    //unsigned long address = (currentNIC->registers_base_address + 0x3810); //head
    //unsigned long address = (currentNIC->registers_base_address + REG_TDT); // tail
    //unsigned int value32 = (uint16_t) *( (volatile unsigned int *) address );
    //currentNIC->tx_cur = (uint16_t) (value32 & 0xFFFF);
    uint16_t buffer_index = (uint16_t) currentNIC->tx_cur;

    //#debug
    //printf ("buffer_index {%d}\n",buffer_index);

//
// Buffer
//

// Let's call it 'frame'.
// Because we're sending a frame.

// Get the buffer address based on its offset.
    unsigned char *frame = 
        (unsigned char *) currentNIC->tx_buffers_virt[buffer_index];

// Get the addresses for the headers.
    unsigned char *src_ethernet = (unsigned char *) eh;
    unsigned char *src_arp      = (unsigned char *) h;


//
// Copy
//

    if ( (void*) frame == NULL )
        panic("network_send_arp: frame\n");

// Copy the ethernet header into the buffer.
// 14 bytes.
    for (i=0; i<ETHERNET_HEADER_LENGHT; i++){
        frame[i] = (unsigned char) src_ethernet[i];
    };
// Copy the arp header into the buffer.
// 28 bytes
// It starts right after the ethernet header.
    for (i=0; i<ARP_HEADER_LENGHT; i++){
        frame[ETHERNET_HEADER_LENGHT + i] = (unsigned char) src_arp[i]; 
    };

// ======================
// Sending via e1000 api.
    // 14 + 28 = 42
    size_t ARP_TOTAL_SIZE = 
        ( ETHERNET_HEADER_LENGHT + \
          ARP_HEADER_LENGHT );
// Sending a frame!
    e1000_send( currentNIC, ARP_TOTAL_SIZE, frame );
    printf ("Done\n");
    refresh_screen();
    return;

fail:
    refresh_screen();
    return;
}


void network_send_arp_request(void)
{
// Send ARP request to 192.168.1.6.

    network_send_arp( 
        ARP_OPC_REQUEST, 
        __arp_gramado_default_ipv4,  // src ip 
        __arp_target_default_ipv4,   // dst ip (Linux)
        __arp_broadcast_mac          // target mac
        );
}


void network_send_arp_reply(void)
{
// Send ARP reply to 192.168.1.6.

// #bugbug
// Sending reply to broadcast

    network_send_arp( 
        ARP_OPC_REPLY, 
        __arp_gramado_default_ipv4,  // src ip 
        __arp_target_default_ipv4,   // dst ip (Linux)
        __arp_broadcast_mac          // target mac
        );
}





