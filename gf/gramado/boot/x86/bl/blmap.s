
Allocating common symbols
Common symbol       size              file

g_fat16_fat_status  0x4               main.o
timerTicks          0x4               timer.o
keyboard_queue_head
                    0x4               main.o
prompt              0xfa              main.o
EDITBOX_LARGURA     0x4               main.o
partition           0x10              main.o
system_info         0x8               main.o
dma_addr            0x4               ide.o
g_initialized       0x4               main.o
ata_record_dev      0x1               main.o
bliTesting          0x4               main.o
g_driver_hdd_initialized
                    0x4               main.o
EDITBOX_BG_COLOR    0x4               main.o
shell_status        0x4               shell.o
current_dev         0x4               ide.o
STATUS_X            0x4               main.o
prompt_pos          0x4               main.o
ide_handler_address
                    0x4               main.o
g_fat16_root_status
                    0x4               main.o
current_mmblock     0x4               main.o
STATUS_Y            0x4               main.o
mm_prev_pointer     0x4               heap.o
shell_string_buffer
                    0x400             shell.o
deslocamento        0x4               shell.o
bl_heap_start       0x4               main.o
buffer_dir_entry    0x200             main.o
BootLoaderCR3       0x4               main.o
next_address        0x4               main.o
shift_status        0x4               keyboard.o
ide_dma_prdt        0x20              ide.o
g_available_heap    0x4               main.o
alt_status          0x4               keyboard.o
ata_record_channel  0x1               main.o
heapCount           0x4               main.o
EDITBOX_ALTURA      0x4               main.o
keyboard_queue_tail
                    0x4               main.o
g_lbf_va            0x4               main.o
LegacyCR3           0x4               main.o
g_proc_status       0x4               main.o
g_current_ide_channel
                    0x4               main.o
g_heap_pointer      0x4               main.o
gdefTesting         0x4               main.o
heapList            0x20              main.o
ata_pci             0x34              main.o
ATAFlag             0x4               main.o
procedure_info      0x10              main.o
EDITBOX_TEXT_COLOR  0x4               main.o
____testing_memory_size_flag
                    0x4               main.o
MENU                0x700             main.o
hddError            0x4               hdd.o
keyboard_flag       0x4               main.o
ide_ports           0x60              main.o
CURSOR_Y            0x4               main.o
EDITBOX_Y           0x4               main.o
IDE                 0x14              main.o
g_system_color      0x4               main.o
CURSOR_X            0x4               main.o
keyboard_queue      0x8               main.o
last_size           0x4               heap.o
useGUI              0x4               main.o
ready_queue_dev     0x4               ide.o
last_valid          0x4               heap.o
ret_string          0x4               shell.o
file_cluster_list   0x800             main.o
vesa_mode           0x10              main.o
escape_status       0x4               keyboard.o
mmblockList         0x400             main.o
ata_identify_dev_buf
                    0x4               main.o
g_next_app          0x4               main.o
bl_heap_end         0x4               main.o
dev_nport           0x20              main.o
__last_valid_address
                    0x4               main.o
ata                 0x18              main.o
tab_status          0x4               keyboard.o
VideoBlock          0x1c              main.o
g_cursor_y          0x4               main.o
idechannelList      0xc0              main.o
g_cmd_status        0x4               main.o
g_current_ide_device
                    0x4               main.o
ShellInitialized    0x4               shell.o
key_status          0x4               keyboard.o
g_file_system_type  0x4               main.o
mmblockCount        0x4               main.o
hddStatus           0x4               hdd.o
g_char_attrib       0x4               main.o
ctrl_status         0x4               keyboard.o
g_next_proc         0x4               main.o
g_lbf_pa            0x4               main.o
winkey_status       0x4               keyboard.o
menu_highlight      0x4               main.o
g_nova_mensagem     0x4               main.o
EDITBOX_X           0x4               main.o
g_cursor_x          0x4               main.o

Memory Configuration

Name             Origin             Length             Attributes
*default*        0x0000000000000000 0xffffffffffffffff

Linker script and memory map

                0x0000000000400000                __image_base__ = 0x400000
                0x0000000000000000                __dll__ = 0x0
                0x0000000000400000                ___ImageBase = 0x400000
                0x0000000000001000                __section_alignment__ = 0x1000
                0x0000000000000200                __file_alignment__ = 0x200
                0x0000000000000004                __major_os_version__ = 0x4
                0x0000000000000000                __minor_os_version__ = 0x0
                0x0000000000000001                __major_image_version__ = 0x1
                0x0000000000000000                __minor_image_version__ = 0x0
                0x0000000000000004                __major_subsystem_version__ = 0x4
                0x0000000000000000                __minor_subsystem_version__ = 0x0
                0x0000000000000003                __subsystem__ = 0x3
                0x0000000000200000                __size_of_stack_reserve__ = 0x200000
                0x0000000000001000                __size_of_stack_commit__ = 0x1000
                0x0000000000100000                __size_of_heap_reserve__ = 0x100000
                0x0000000000001000                __size_of_heap_commit__ = 0x1000
                0x0000000000000000                __loader_flags__ = 0x0
                0x0000000000000000                __dll_characteristics__ = 0x0

.text           0x0000000000021000     0x7000
                0x0000000000021000                code = .
                0x0000000000021000                _code = .
                0x0000000000021000                __code = .
 *(.head_x86)
 .head_x86      0x0000000000021000     0x140e head.o
                0x0000000000021000                bootloader_entry_point
                0x00000000000211fd                SavedBootBlock
                0x0000000000021201                SavedLFB
                0x0000000000021205                SavedX
                0x0000000000021209                SavedY
                0x000000000002120d                SavedBPP
                0x0000000000021211                SavedBootMode
                0x0000000000021b99                task0_stack
                0x0000000000021b9d                task1_stack
                0x0000000000021ba1                task2_stack
                0x0000000000021bd9                setup_idt_vector
                0x0000000000021e71                irq0
                0x0000000000021e77                timer_test
                0x0000000000021e7f                irq1
                0x0000000000021eab                fault_N0
                0x0000000000021eb5                fault_N1
                0x0000000000021ebf                fault_N2
                0x0000000000021ec9                fault_N3
                0x0000000000021ed3                fault_N4
                0x0000000000021edd                fault_N5
                0x0000000000021ee7                fault_INTRUCAO_INVALIDA
                0x0000000000021ef1                fault_N7
                0x0000000000021efb                fault_DOUBLE
                0x0000000000021f05                fault_N9
                0x0000000000021f0f                fault_N10
                0x0000000000021f19                fault_N11
                0x0000000000021f23                fault_STACK
                0x0000000000021f2d                fault_GP
                0x0000000000021f37                fault_N14
                0x0000000000021f41                fault_N15
                0x0000000000021f4b                fault_N16
                0x0000000000021f55                fault_N17
                0x0000000000021f5f                fault_N18
                0x0000000000021f69                fault_N19
                0x0000000000021f73                fault_N20
                0x0000000000021f7d                fault_N21
                0x0000000000021f87                fault_N22
                0x0000000000021f91                fault_N23
                0x0000000000021f9b                fault_N24
                0x0000000000021fa5                fault_N25
                0x0000000000021faf                fault_N26
                0x0000000000021fb9                fault_N27
                0x0000000000021fc3                fault_N28
                0x0000000000021fcd                fault_N29
                0x0000000000021fd7                fault_N30
                0x0000000000021fe1                fault_N31
                0x0000000000021feb                hd_buffer
                0x0000000000021fef                hd_lba
                0x0000000000021ff3                test_write
                0x0000000000022003                os_read_sector
                0x000000000002201c                os_write_sector
                0x000000000002212b                reset_ide0
                0x00000000000221ca                int48
                0x00000000000221cf                int49
                0x00000000000221d4                int50
                0x00000000000221d9                int51
                0x00000000000221de                int52
                0x00000000000221e3                int53
                0x00000000000221e8                int54
                0x00000000000221ed                int55
                0x00000000000221f2                int56
                0x00000000000221f7                int57
                0x00000000000221fc                int58
                0x0000000000022201                int59
                0x0000000000022206                int100
                0x000000000002220b                int200
                0x000000000002222a                int201
                0x000000000002222f                int213
                0x0000000000022236                my_buffer_load_bitmap_16x16
                0x00000000000222c8                background
                0x000000000002231d                gui_buffer_putpixel
                0x00000000000223ab                refresh_screen
                0x00000000000223fc                softwarelibReboot
                0x000000000002240a                asm_shut_down
 *(.text)
 .text          0x000000000002240e      0x3ce main.o
                0x000000000002240e                blShowMenu
                0x00000000000224c4                BlMenu
                0x000000000002258d                BlMain
                0x0000000000022629                BlLoadKernel
                0x000000000002265c                BlSetupPaging
                0x000000000002266a                BlAbort
                0x0000000000022678                BlKernelModuleMain
                0x0000000000022696                init_testing_memory_size
                0x00000000000227bd                die
 .text          0x00000000000227dc      0x127 loader.o
                0x00000000000227dc                load_kernel
 .text          0x0000000000022903      0x107 init.o
                0x0000000000022903                set_up_color
                0x0000000000022911                set_up_text_color
                0x000000000002293d                init_globals
                0x0000000000022975                init
 .text          0x0000000000022a0a       0x7d ports.o
                0x0000000000022a0a                in8
                0x0000000000022a1f                in16
                0x0000000000022a36                in32
                0x0000000000022a4a                out8
                0x0000000000022a61                out16
                0x0000000000022a7a                out32
 .text          0x0000000000022a87       0x6f x86.o
                0x0000000000022a87                enable
                0x0000000000022a8e                disable
                0x0000000000022a95                stopCpu
                0x0000000000022a9d                intReturn
                0x0000000000022aa4                farReturn
                0x0000000000022aab                getFlags
                0x0000000000022ab6                setFlags
                0x0000000000022ac1                Push
                0x0000000000022ac7                Pop
                0x0000000000022acd                pushRegs
                0x0000000000022ad3                popRegs
                0x0000000000022ad9                pushFlags
                0x0000000000022adf                popFlags
                0x0000000000022ae5                getStackPointer
                0x0000000000022af0                setStackPointer
 .text          0x0000000000022af6      0xbbb stdio.o
                0x0000000000022af6                panic
                0x0000000000022b17                scroll
                0x0000000000022ba2                bl_clear
                0x0000000000022bfc                kprintf
                0x000000000002304f                printf
                0x000000000002306c                sprintf
                0x00000000000230c1                putchar
                0x00000000000230da                outbyte
                0x0000000000023241                _outbyte
                0x000000000002332c                printf_main
                0x0000000000023336                input
                0x0000000000023465                my_buffer_horizontal_line
                0x000000000002348f                my_buffer_put_pixel
                0x0000000000023568                my_buffer_char_blt
                0x00000000000235e4                vsync
                0x0000000000023621                gui_inb
                0x0000000000023646                clear_backbuffer
                0x000000000002367e                get_cursor_x
                0x0000000000023688                get_cursor_y
                0x0000000000023692                carrega_bitmap_16x16
 .text          0x00000000000236b1       0x5b stdlib.o
                0x00000000000236b1                malloc
                0x00000000000236ed                free
 .text          0x000000000002370c      0x25f string.o
                0x000000000002370c                strcmp
                0x0000000000023778                strncmp
                0x00000000000237db                str_cmp
                0x0000000000023858                memcpy
                0x0000000000023895                strlen
                0x00000000000238bc                strcpy
                0x00000000000238f0                strcat
                0x000000000002391f                bcopy
                0x000000000002394b                bzero
 .text          0x000000000002396b      0x284 keyboard.o
                0x000000000002396b                keyboardHandler
                0x0000000000023b93                keyboad_get_char
                0x0000000000023bda                keyboard_wait_key
 .text          0x0000000000023bef      0x27f pci.o
                0x0000000000023bef                pciConfigReadWord
                0x0000000000023c9d                pciCheckDevice
                0x0000000000023d03                pciCheckVendor
                0x0000000000023d49                pciGetClassCode
                0x0000000000023d7a                pciInfo
                0x0000000000023e64                pciInit
 .text          0x0000000000023e6e      0x469 hdd.o
                0x0000000000023ea4                hdd_ata_pio_write
                0x0000000000023eda                hdd_ata_status_read
                0x0000000000023f08                hdd_ata_wait_not_busy
                0x0000000000023f48                hdd_ata_cmd_write
                0x0000000000023f9c                hdd_ata_wait_no_drq
                0x0000000000023fe2                pio_rw_sector
                0x0000000000024257                my_read_hd_sector
                0x0000000000024280                my_write_hd_sector
                0x00000000000242a9                init_hdd
 .text          0x00000000000242d7     0x1c94 ide.o
                0x00000000000242d7                disk_get_ata_irq_invoked
                0x00000000000242e1                disk_reset_ata_irq_invoked
                0x00000000000242f1                ata_wait
                0x0000000000024323                ata_wait_not_busy
                0x0000000000024351                ata_wait_busy
                0x000000000002437f                ata_wait_no_drq
                0x00000000000243b3                ata_wait_drq
                0x00000000000243e7                ata_wait_irq
                0x000000000002445e                ata_soft_reset
                0x00000000000244b7                ata_status_read
                0x00000000000244d3                ata_cmd_write
                0x000000000002450d                ata_assert_dever
                0x00000000000245b2                ide_identify_device
                0x0000000000024a0a                set_ata_addr
                0x0000000000024a60                ide_mass_storage_initialize
                0x0000000000024b0c                ide_dev_init
                0x0000000000024ed8                nport_ajuste
                0x0000000000024f30                ata_pio_read
                0x0000000000024f55                ata_pio_write
                0x0000000000024f7a                ide_dma_data
                0x0000000000025060                ide_dma_start
                0x000000000002509a                ide_dma_stop
                0x0000000000025109                ide_dma_read_status
                0x0000000000025128                diskReadPCIConfigAddr
                0x0000000000025187                diskWritePCIConfigAddr
                0x00000000000251eb                diskATAPCIConfigurationSpace
                0x00000000000258c2                diskPCIScanDevice
                0x00000000000259e4                diskATAInitialize
                0x0000000000025d5f                diskATADialog
                0x0000000000025da3                diskATAIRQHandler1
                0x0000000000025db3                diskATAIRQHandler2
                0x0000000000025dc3                show_ide_info
                0x0000000000025eed                disk_ata_wait_irq
 .text          0x0000000000025f6b       0x48 timer.o
                0x0000000000025f6b                timer
                0x0000000000025f97                BltimerInit
 .text          0x0000000000025fb3      0x588 pages.o
                0x0000000000025fb3                SetUpPaging
 .text          0x000000000002653b      0x3b7 heap.o
                0x000000000002653b                heapAllocateMemory
                0x0000000000026716                FreeHeap
                0x00000000000267d6                init_heap
 .text          0x00000000000268f2       0x95 procedure.o
                0x00000000000268f2                bl_procedure
 .text          0x0000000000026987      0xbaf fs.o
                0x0000000000026987                fatClustToSect
                0x00000000000269ae                fatLoadCluster
                0x00000000000269f1                fs_format
                0x00000000000269f7                fs_search_empty_entry
                0x0000000000026a01                fs_find_not_empty_entry
                0x0000000000026a0b                fs_test_fat_vector
                0x0000000000026a11                fs_get_fat_entry
                0x0000000000026a1b                fs_create_entry
                0x0000000000026a21                fsCreateFileName
                0x0000000000026a27                fsSaveFileName
                0x0000000000026a2d                fs_save_entry_on_disc
                0x0000000000026a33                fs_set_structures
                0x0000000000026a39                fs_set_entry
                0x0000000000026a3f                fs_get_entry
                0x0000000000026a45                fs_show_dir_entry
                0x0000000000026a4b                fs_show_dir
                0x0000000000026a51                fs_check_cluster
                0x0000000000026a5b                fsSaveRootDir
                0x0000000000026a61                fs_load_dir
                0x0000000000026a67                fs_save_dir
                0x0000000000026a6d                fs_save_structure
                0x0000000000026a73                fs_save_entry_on_root
                0x0000000000026a79                fs_show_entry
                0x0000000000026a7f                fs_delete_entry
                0x0000000000026a85                fs_init_bootfile_struct
                0x0000000000026a8b                fs_get_entry_status
                0x0000000000026a95                fs_set_entry_status
                0x0000000000026a9b                fs_makeboot
                0x0000000000026aa5                fs_set_fat_entry
                0x0000000000026aab                fs_find_empty_entry
                0x0000000000026ab5                fs_install
                0x0000000000026abb                fs_relood_dir
                0x0000000000026ac1                fsLoadFile
                0x0000000000026ca6                path_count
                0x0000000000026d00                load_path
                0x000000000002701e                fsSearchFile
                0x00000000000270f7                fsSearchFileName
                0x000000000002710d                fs_load_rootdirEx
                0x0000000000027165                fs_load_fatEx
                0x00000000000271ba                fs_put_list_on_fat
                0x0000000000027283                fs_find_n_empty_entries
                0x0000000000027318                fs_load_rootdir
                0x0000000000027326                read_lba
                0x0000000000027341                write_lba
                0x000000000002735c                fsSaveFile
                0x0000000000027366                fsCreateDir
                0x000000000002737f                fsCreateFile
                0x0000000000027488                fsClearFat
                0x0000000000027496                fsCheckFat
                0x0000000000027516                fsInitFat
                0x000000000002751c                fsInitStructures
                0x0000000000027522                fsInit
 .text          0x0000000000027536      0x607 shell.o
                0x0000000000027536                blShellMain
                0x00000000000275a1                shellProcedure
                0x0000000000027612                shellInitializePrompt
                0x00000000000276a1                shellWaitCmd
                0x00000000000276bb                shellCompare
                0x0000000000027a6f                shellHelp
                0x0000000000027a88                boot
                0x0000000000027a8e                debug
                0x0000000000027aa9                testa_mbr
                0x0000000000027ada                testa_root
                0x0000000000027b0e                reboot
                0x0000000000027b1c                shellInit
 .text          0x0000000000027b3d       0x17 services.o
                0x0000000000027b3d                blServices
                0x0000000000027b4e                system_services
 .text          0x0000000000027b54       0x1f abort.o
                0x0000000000027b54                abort
 .text          0x0000000000027b73      0x10c faults.o
                0x0000000000027b73                cpu_faults
                0x0000000000027c79                faultsShowRegisters
                0x0000000000028000                . = ALIGN (0x1000)
 *fill*         0x0000000000027c7f      0x381 

.rodata         0x0000000000028000      0xedc
 .rodata        0x0000000000028000      0x163 main.o
 *fill*         0x0000000000028163        0x1 
 .rodata        0x0000000000028164       0xaf loader.o
 .rodata        0x0000000000028213       0x11 init.o
 .rodata        0x0000000000028224       0x3f stdio.o
 *fill*         0x0000000000028263        0x1 
 .rodata        0x0000000000028264       0x44 pci.o
 .rodata        0x00000000000282a8       0x33 hdd.o
 *fill*         0x00000000000282db        0x1 
 .rodata        0x00000000000282dc      0x46b ide.o
 *fill*         0x0000000000028747        0x1 
 .rodata        0x0000000000028748      0x1b1 heap.o
 .rodata        0x00000000000288f9        0xc procedure.o
 *fill*         0x0000000000028905        0x3 
 .rodata        0x0000000000028908      0x324 fs.o
 .rodata        0x0000000000028c2c      0x196 shell.o
 .rodata        0x0000000000028dc2        0xd abort.o
 *fill*         0x0000000000028dcf        0x1 
 .rodata        0x0000000000028dd0      0x10c faults.o

.eh_frame       0x0000000000029000     0x1b60
 .eh_frame      0x0000000000029000      0x134 main.o
 .eh_frame      0x0000000000029134       0x38 loader.o
 .eh_frame      0x000000000002916c       0x98 init.o
 .eh_frame      0x0000000000029204       0xd8 ports.o
 .eh_frame      0x00000000000292dc      0x1f8 x86.o
 .eh_frame      0x00000000000294d4      0x34c stdio.o
 .eh_frame      0x0000000000029820       0x58 stdlib.o
 .eh_frame      0x0000000000029878      0x138 string.o
 .eh_frame      0x00000000000299b0       0x78 keyboard.o
 .eh_frame      0x0000000000029a28       0xe0 pci.o
 .eh_frame      0x0000000000029b08      0x160 hdd.o
 .eh_frame      0x0000000000029c68      0x494 ide.o
 .eh_frame      0x000000000002a0fc       0x58 timer.o
 .eh_frame      0x000000000002a154       0x38 pages.o
 .eh_frame      0x000000000002a18c       0x78 heap.o
 .eh_frame      0x000000000002a204       0x38 procedure.o
 .eh_frame      0x000000000002a23c      0x698 fs.o
 .eh_frame      0x000000000002a8d4      0x198 shell.o
 .eh_frame      0x000000000002aa6c       0x58 services.o
 .eh_frame      0x000000000002aac4       0x34 abort.o
 .eh_frame      0x000000000002aaf8       0x58 faults.o

.data           0x000000000002ab60     0x14a0
                0x000000000002ab60                data = .
                0x000000000002ab60                _data = .
                0x000000000002ab60                __data = .
 *(.data)
 .data          0x000000000002ab60        0x2 head.o
                0x000000000002ab60                data_start
 .data          0x000000000002ab62        0x0 main.o
 .data          0x000000000002ab62        0x0 loader.o
 .data          0x000000000002ab62        0x0 init.o
 .data          0x000000000002ab62        0x0 ports.o
 .data          0x000000000002ab62        0x0 x86.o
 .data          0x000000000002ab62        0x0 stdio.o
 .data          0x000000000002ab62        0x0 stdlib.o
 .data          0x000000000002ab62        0x0 string.o
 *fill*         0x000000000002ab62       0x1e 
 .data          0x000000000002ab80      0x1a0 keyboard.o
                0x000000000002ab80                destroy_window
                0x000000000002ab84                quit_message
 .data          0x000000000002ad20        0x0 pci.o
 .data          0x000000000002ad20        0x0 hdd.o
 .data          0x000000000002ad20      0x440 ide.o
                0x000000000002ad3c                dev_type
                0x000000000002ad60                pci_classes
 .data          0x000000000002b160        0x0 timer.o
 .data          0x000000000002b160        0x0 pages.o
 .data          0x000000000002b160        0x0 heap.o
 .data          0x000000000002b160        0x0 procedure.o
 .data          0x000000000002b160        0x0 fs.o
 .data          0x000000000002b160       0x2c shell.o
                0x000000000002b160                help_string
 .data          0x000000000002b18c        0x0 services.o
 .data          0x000000000002b18c        0x0 abort.o
 .data          0x000000000002b18c        0x0 faults.o
                0x000000000002c000                . = ALIGN (0x1000)
 *fill*         0x000000000002b18c      0xe74 

.bss            0x000000000002c000     0x3000
                0x000000000002c000                bss = .
                0x000000000002c000                _bss = .
                0x000000000002c000                __bss = .
 *(.bss)
 .bss           0x000000000002c000        0x0 head.o
                0x000000000002c000                bss_start
 .bss           0x000000000002c000        0x0 main.o
 .bss           0x000000000002c000        0x0 loader.o
 .bss           0x000000000002c000        0x0 init.o
 .bss           0x000000000002c000        0x0 ports.o
 .bss           0x000000000002c000        0x0 x86.o
 .bss           0x000000000002c000        0x1 stdio.o
 .bss           0x000000000002c001        0x0 stdlib.o
 .bss           0x000000000002c001        0x0 string.o
 *fill*         0x000000000002c001        0x3 
 .bss           0x000000000002c004        0x4 keyboard.o
                0x000000000002c004                ambiente
 .bss           0x000000000002c008        0x0 pci.o
 .bss           0x000000000002c008        0x0 hdd.o
 .bss           0x000000000002c008       0x20 ide.o
                0x000000000002c008                dev_next_pid
 .bss           0x000000000002c028        0x0 timer.o
 .bss           0x000000000002c028        0x0 pages.o
 .bss           0x000000000002c028        0x0 heap.o
 .bss           0x000000000002c028        0x0 procedure.o
 .bss           0x000000000002c028        0x0 fs.o
 .bss           0x000000000002c028        0x0 shell.o
 .bss           0x000000000002c028        0x0 services.o
 .bss           0x000000000002c028        0x0 abort.o
 .bss           0x000000000002c028        0x0 faults.o
 *(COMMON)
 *fill*         0x000000000002c028       0x18 
 COMMON         0x000000000002c040     0x198c main.o
                0x000000000002c040                g_fat16_fat_status
                0x000000000002c044                keyboard_queue_head
                0x000000000002c060                prompt
                0x000000000002c15c                EDITBOX_LARGURA
                0x000000000002c160                partition
                0x000000000002c170                system_info
                0x000000000002c178                g_initialized
                0x000000000002c17c                ata_record_dev
                0x000000000002c180                bliTesting
                0x000000000002c184                g_driver_hdd_initialized
                0x000000000002c188                EDITBOX_BG_COLOR
                0x000000000002c18c                STATUS_X
                0x000000000002c190                prompt_pos
                0x000000000002c194                ide_handler_address
                0x000000000002c198                g_fat16_root_status
                0x000000000002c19c                current_mmblock
                0x000000000002c1a0                STATUS_Y
                0x000000000002c1a4                bl_heap_start
                0x000000000002c1c0                buffer_dir_entry
                0x000000000002c3c0                BootLoaderCR3
                0x000000000002c3c4                next_address
                0x000000000002c3c8                g_available_heap
                0x000000000002c3cc                ata_record_channel
                0x000000000002c3d0                heapCount
                0x000000000002c3d4                EDITBOX_ALTURA
                0x000000000002c3d8                keyboard_queue_tail
                0x000000000002c3dc                g_lbf_va
                0x000000000002c3e0                LegacyCR3
                0x000000000002c3e4                g_proc_status
                0x000000000002c3e8                g_current_ide_channel
                0x000000000002c3ec                g_heap_pointer
                0x000000000002c3f0                gdefTesting
                0x000000000002c400                heapList
                0x000000000002c420                ata_pci
                0x000000000002c454                ATAFlag
                0x000000000002c458                procedure_info
                0x000000000002c468                EDITBOX_TEXT_COLOR
                0x000000000002c46c                ____testing_memory_size_flag
                0x000000000002c480                MENU
                0x000000000002cb80                keyboard_flag
                0x000000000002cba0                ide_ports
                0x000000000002cc00                CURSOR_Y
                0x000000000002cc04                EDITBOX_Y
                0x000000000002cc08                IDE
                0x000000000002cc1c                g_system_color
                0x000000000002cc20                CURSOR_X
                0x000000000002cc24                keyboard_queue
                0x000000000002cc2c                useGUI
                0x000000000002cc40                file_cluster_list
                0x000000000002d440                vesa_mode
                0x000000000002d460                mmblockList
                0x000000000002d860                ata_identify_dev_buf
                0x000000000002d864                g_next_app
                0x000000000002d868                bl_heap_end
                0x000000000002d880                dev_nport
                0x000000000002d8a0                __last_valid_address
                0x000000000002d8a4                ata
                0x000000000002d8bc                VideoBlock
                0x000000000002d8d8                g_cursor_y
                0x000000000002d8e0                idechannelList
                0x000000000002d9a0                g_cmd_status
                0x000000000002d9a4                g_current_ide_device
                0x000000000002d9a8                g_file_system_type
                0x000000000002d9ac                mmblockCount
                0x000000000002d9b0                g_char_attrib
                0x000000000002d9b4                g_next_proc
                0x000000000002d9b8                g_lbf_pa
                0x000000000002d9bc                menu_highlight
                0x000000000002d9c0                g_nova_mensagem
                0x000000000002d9c4                EDITBOX_X
                0x000000000002d9c8                g_cursor_x
 COMMON         0x000000000002d9cc       0x1c keyboard.o
                0x000000000002d9cc                shift_status
                0x000000000002d9d0                alt_status
                0x000000000002d9d4                escape_status
                0x000000000002d9d8                tab_status
                0x000000000002d9dc                key_status
                0x000000000002d9e0                ctrl_status
                0x000000000002d9e4                winkey_status
 COMMON         0x000000000002d9e8        0x8 hdd.o
                0x000000000002d9e8                hddError
                0x000000000002d9ec                hddStatus
 *fill*         0x000000000002d9f0       0x10 
 COMMON         0x000000000002da00       0x44 ide.o
                0x000000000002da00                dma_addr
                0x000000000002da04                current_dev
                0x000000000002da20                ide_dma_prdt
                0x000000000002da40                ready_queue_dev
 COMMON         0x000000000002da44        0x4 timer.o
                0x000000000002da44                timerTicks
 COMMON         0x000000000002da48        0xc heap.o
                0x000000000002da48                mm_prev_pointer
                0x000000000002da4c                last_size
                0x000000000002da50                last_valid
 *fill*         0x000000000002da54        0xc 
 COMMON         0x000000000002da60      0x42c shell.o
                0x000000000002da60                shell_status
                0x000000000002da80                shell_string_buffer
                0x000000000002de80                deslocamento
                0x000000000002de84                ret_string
                0x000000000002de88                ShellInitialized
 *(.bss_heap)
 .bss_heap      0x000000000002de8c      0x400 head.o
                0x000000000002de8c                bootloader_heap_start
                0x000000000002e28c                bootloader_heap_end
 *(.bss_stack)
 .bss_stack     0x000000000002e28c      0x400 head.o
                0x000000000002e28c                bootloader_stack_end
                0x000000000002e68c                bootloader_stack_start
                0x000000000002f000                . = ALIGN (0x1000)
 *fill*         0x000000000002e68c      0x974 
                0x000000000002f000                end = .
                0x000000000002f000                _end = .
                0x000000000002f000                __end = .
                0x000000000002f000                _bootloader_end = .
LOAD head.o
LOAD main.o
LOAD loader.o
LOAD init.o
LOAD ports.o
LOAD x86.o
LOAD stdio.o
LOAD stdlib.o
LOAD string.o
LOAD keyboard.o
LOAD pci.o
LOAD hdd.o
LOAD ide.o
LOAD timer.o
LOAD pages.o
LOAD heap.o
LOAD procedure.o
LOAD fs.o
LOAD shell.o
LOAD services.o
LOAD abort.o
LOAD faults.o
OUTPUT(BL.BIN pe-i386)

.comment        0x0000000000000000      0x374
 .comment       0x0000000000000000       0x2a main.o
 .comment       0x000000000000002a       0x2a loader.o
 .comment       0x0000000000000054       0x2a init.o
 .comment       0x000000000000007e       0x2a ports.o
 .comment       0x00000000000000a8       0x2a x86.o
 .comment       0x00000000000000d2       0x2a stdio.o
 .comment       0x00000000000000fc       0x2a stdlib.o
 .comment       0x0000000000000126       0x2a string.o
 .comment       0x0000000000000150       0x2a keyboard.o
 .comment       0x000000000000017a       0x2a pci.o
 .comment       0x00000000000001a4       0x2a hdd.o
 .comment       0x00000000000001ce       0x2a ide.o
 .comment       0x00000000000001f8       0x2a timer.o
 .comment       0x0000000000000222       0x2a pages.o
 .comment       0x000000000000024c       0x2a heap.o
 .comment       0x0000000000000276       0x2a procedure.o
 .comment       0x00000000000002a0       0x2a fs.o
 .comment       0x00000000000002ca       0x2a shell.o
 .comment       0x00000000000002f4       0x2a services.o
 .comment       0x000000000000031e       0x2a abort.o
 .comment       0x0000000000000348       0x2a faults.o

.note.GNU-stack
                0x0000000000000000        0x0
 .note.GNU-stack
                0x0000000000000000        0x0 main.o
 .note.GNU-stack
                0x0000000000000000        0x0 loader.o
 .note.GNU-stack
                0x0000000000000000        0x0 init.o
 .note.GNU-stack
                0x0000000000000000        0x0 ports.o
 .note.GNU-stack
                0x0000000000000000        0x0 x86.o
 .note.GNU-stack
                0x0000000000000000        0x0 stdio.o
 .note.GNU-stack
                0x0000000000000000        0x0 stdlib.o
 .note.GNU-stack
                0x0000000000000000        0x0 string.o
 .note.GNU-stack
                0x0000000000000000        0x0 keyboard.o
 .note.GNU-stack
                0x0000000000000000        0x0 pci.o
 .note.GNU-stack
                0x0000000000000000        0x0 hdd.o
 .note.GNU-stack
                0x0000000000000000        0x0 ide.o
 .note.GNU-stack
                0x0000000000000000        0x0 timer.o
 .note.GNU-stack
                0x0000000000000000        0x0 pages.o
 .note.GNU-stack
                0x0000000000000000        0x0 heap.o
 .note.GNU-stack
                0x0000000000000000        0x0 procedure.o
 .note.GNU-stack
                0x0000000000000000        0x0 fs.o
 .note.GNU-stack
                0x0000000000000000        0x0 shell.o
 .note.GNU-stack
                0x0000000000000000        0x0 services.o
 .note.GNU-stack
                0x0000000000000000        0x0 abort.o
 .note.GNU-stack
                0x0000000000000000        0x0 faults.o
