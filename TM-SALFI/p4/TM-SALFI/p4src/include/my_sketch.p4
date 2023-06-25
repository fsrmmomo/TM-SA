/* -*- P4_16 -*- */

// enum bit<32> crc32_polinomials {
//     COEFF1       = 0x04C11DB7,
//     COEFF2       = 0xEDB88320,
//     COEFF3       = 0xDB710641,
//     COEFF4       = 0x82608EDB,
//     COEFF5       = 0x741B8CD7,
//     COEFF6       = 0xEB31D82E,
//     COEFF7       = 0xD663B050,
//     COEFF8       = 0xBA0DC66B
// }

// control calc_ipv4_hash_ingress(
//     in       my_ingress_headers_t   hdr,
//     inout    my_ingress_metadata_t  meta)
// {
//     CRCPolynomial<bit<32>>(
//         coeff    = 0x04C11DB7,
//         reversed = true,
//         msb      = false,
//         extended = false,
//         init     = 0xFFFFFFFF,
//         xor      = 0xFFFFFFFF) poly1;
//     Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly1) hash_algo1;

//     CRCPolynomial<bit<32>>(
//         coeff    = 0x741B8CD7,
//         reversed = true,
//         msb      = false,
//         extended = false,
//         init     = 0xFFFFFFFF,
//         xor      = 0xFFFFFFFF) poly2;
//     Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly2) hash_algo2;

//     CRCPolynomial<bit<32>>(
//         coeff    = 0xDB710641,
//         reversed = true,
//         msb      = false,
//         extended = false,
//         init     = 0xFFFFFFFF,
//         xor      = 0xFFFFFFFF) poly3;
//     Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly3) hash_algo3;

//     action do_hash1() {
//         meta.index_sketch1 = hash_algo1.get({
//                 hdr.ipv4.src_addr,
//                 hdr.ipv4.dst_addr,
//                 hdr.ipv4.protocol,
//                 meta.l4_lookup.src_port,
//                 meta.l4_lookup.dst_port
//             });
//     }

//     action do_hash2() {
//         meta.index_sketch2 = hash_algo2.get({
//                 hdr.ipv4.src_addr,
//                 hdr.ipv4.dst_addr,
//                 hdr.ipv4.protocol,
//                 meta.l4_lookup.src_port,
//                 meta.l4_lookup.dst_port
//             });
//     }

//     action do_hash3() {
//         meta.index_sketch3 = hash_algo3.get({
//                 hdr.ipv4.src_addr,
//                 hdr.ipv4.dst_addr,
//                 hdr.ipv4.protocol,
//                 meta.l4_lookup.src_port,
//                 meta.l4_lookup.dst_port
//             });
//     }

//     apply {
//         do_hash1();
//         do_hash2();
//         do_hash3();
//     }
// }

/* CM Sketch  600 KB*/
const int SKETCH_TOTAL_MEM = 900 * 1024;
const int SKETCH_HEIGHT = 3;
const int SKETCH_BUCKET_LENGTH = SKETCH_TOTAL_MEM / SKETCH_HEIGHT;

const int SKETCH_BUCKET_LENGTH_WIDTH = 20;
typedef bit<(SKETCH_BUCKET_LENGTH_WIDTH)> SKETCH_BUCKET_LENGTH_t;
// const int SKETCH_BUCKET_LENGTH = 1 << (SKETCH_BUCKET_LENGTH_WIDTH);


const int SKETCH_COUNT_BIT_WIDTH = 8;
typedef bit<(SKETCH_COUNT_BIT_WIDTH)> SKETCH_COUNT_BIT_WIDTH_t;

control cm_sketch_ingress(
    inout  TM_SALFI_ingress_metadata_t   meta)
{

    Register<SKETCH_COUNT_BIT_WIDTH_t, SKETCH_BUCKET_LENGTH_t>(SKETCH_BUCKET_LENGTH,0) sketch_count1;
    RegisterAction<SKETCH_COUNT_BIT_WIDTH_t, SKETCH_BUCKET_LENGTH_t, SKETCH_COUNT_BIT_WIDTH_t>(sketch_count1)
    leave_count1 = {
        void apply(inout SKETCH_COUNT_BIT_WIDTH_t register_data) {
            register_data = register_data |+| 1;
        }
    };
    
    Register<SKETCH_COUNT_BIT_WIDTH_t, SKETCH_BUCKET_LENGTH_t>(SKETCH_BUCKET_LENGTH,0) sketch_count2;
    RegisterAction<SKETCH_COUNT_BIT_WIDTH_t, SKETCH_BUCKET_LENGTH_t, SKETCH_COUNT_BIT_WIDTH_t>(sketch_count2)
    leave_count2 = {
        void apply(inout SKETCH_COUNT_BIT_WIDTH_t register_data) {
            register_data = register_data |+| 1;
        }
    };
    
    Register<SKETCH_COUNT_BIT_WIDTH_t, SKETCH_BUCKET_LENGTH_t>(SKETCH_BUCKET_LENGTH,0) sketch_count3;
    RegisterAction<SKETCH_COUNT_BIT_WIDTH_t, SKETCH_BUCKET_LENGTH_t, SKETCH_COUNT_BIT_WIDTH_t>(sketch_count3)
    leave_count3 = {
        void apply(inout SKETCH_COUNT_BIT_WIDTH_t register_data) {
            register_data = register_data |+| 1;
        }
    };



    apply{
        leave_count1.execute(meta.index_sketch1[(SKETCH_BUCKET_LENGTH_WIDTH - 1):0]);
        leave_count2.execute(meta.index_sketch2[(SKETCH_BUCKET_LENGTH_WIDTH - 1):0]);
        leave_count3.execute(meta.index_sketch3[(SKETCH_BUCKET_LENGTH_WIDTH - 1):0]);
    }
}

/* hash table 100 KB*/
const int HASH_TABLE_HEIGHT = 8;
const int HASH_TABLE_WIDTH = 1600;
const int HASH_TABLE_BUCKET_LENGTH_WIDTH_KEY = 32;
const int HASH_TABLE_BUCKET_LENGTH_WIDTH_COUNT = 32;
// typedef bit<(HASH_TABLE_BUCKET_LENGTH)> HASH_TABLE_LENGTH_t;
typedef bit<(HASH_TABLE_BUCKET_LENGTH_WIDTH_KEY)> HASH_TABLE_KEY_BIT_WIDTH_t;
typedef bit<(HASH_TABLE_BUCKET_LENGTH_WIDTH_COUNT)> HASH_TABLE_COUNT_BIT_WIDTH_t;
typedef bit<32> CHECK_FLAG;
const int HASH_TABLE_TOTAL_BUCKETS = 12800;

control hash_tbl_ingress(
    inout  TM_SALFI_ingress_metadata_t   meta)
{
    Register<bit<32>, bit<32>>(1600,0) hash_tbl_key1;
    Register<bit<32>, bit<32>>(1600,0) hash_tbl_key2;
    Register<bit<32>, bit<32>>(1600,0) hash_tbl_key3;
    Register<bit<32>, bit<32>>(1600,0) hash_tbl_key4;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_key5;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_key6;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_key7;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_key8;

    Register<bit<32>, bit<32>>(1600,0) hash_tbl_count1;
    Register<bit<32>, bit<32>>(1600,0) hash_tbl_count2;
    Register<bit<32>, bit<32>>(1600,0) hash_tbl_count3;
    Register<bit<32>, bit<32>>(1600,0) hash_tbl_count4;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_count5;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_count6;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_count7;
    // Register<bit<32>, bit<32>>(1600,0) hash_tbl_count8;


    RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key1)
    check_key1 = {
        void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
            if(register_data != 0){
                if(register_data == meta.flow_id){
                    result = 1;
                }else{
                    result = 0;
                }
            }else{
                result = 1;
                register_data = meta.flow_id;
            }
            // register_data = register_data |+| 1;ls
        }
    };

    RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count1)
    count_packet1 = {
        void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
            register_data = register_data |+| 1;
        }
    };


    RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key2)
    check_key2 = {
        void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
            if(register_data != 0){
                if(register_data == meta.flow_id){
                    result = 1;
                }else{
                    result = 0;
                }
            }else{
                result = 1;
                register_data = meta.flow_id;
            }
            // register_data = register_data |+| 1;
        }
    };

    RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count2)
    count_packet2 = {
        void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
            register_data = register_data |+| 1;
        }
    };

        RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key3)
    check_key3 = {
        void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
            if(register_data != 0){
                if(register_data == meta.flow_id){
                    result = 1;
                }else{
                    result = 0;
                }
            }else{
                result = 1;
                register_data = meta.flow_id;
            }
            // register_data = register_data |+| 1;
        }
    };

    RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count3)
    count_packet3 = {
        void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
            register_data = register_data |+| 1;
        }
    };
    

    RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key4)
    check_key4 = {
        void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
            if(register_data != 0){
                if(register_data == meta.flow_id){
                    result = 1;
                }else{
                    result = 0;
                }
            }else{
                result = 1;
                register_data = meta.flow_id;
            }
            // register_data = register_data |+| 1;
        }
    };

    RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count4)
    count_packet4 = {
        void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
            register_data = register_data |+| 1;
        }
    };
    

    // RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key5)
    // check_key5 = {
    //     void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
    //         if(register_data != 0){
    //             if(register_data == meta.flow_id){
    //                 result = 1;
    //             }else{
    //                 result = 0;
    //             }
    //         }else{
    //             result = 1;
    //             register_data = meta.flow_id;
    //         }
    //         // register_data = register_data |+| 1;
    //     }
    // };

    // RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count5)
    // count_packet5 = {
    //     void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
    //         register_data = register_data |+| 1;
    //     }
    // };
    

    // RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key6)
    // check_key6 = {
    //     void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
    //         if(register_data != 0){
    //             if(register_data == meta.flow_id){
    //                 result = 1;
    //             }else{
    //                 result = 0;
    //             }
    //         }else{
    //             result = 1;
    //             register_data = meta.flow_id;
    //         }
    //         // register_data = register_data |+| 1;
    //     }
    // };

    // RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count6)
    // count_packet6 = {
    //     void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
    //         register_data = register_data |+| 1;
    //     }
    // };
    

    // RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key7)
    // check_key7 = {
    //     void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
    //         if(register_data != 0){
    //             if(register_data == meta.flow_id){
    //                 result = 1;
    //             }else{
    //                 result = 0;
    //             }
    //         }else{
    //             result = 1;
    //             register_data = meta.flow_id;
    //         }
    //         // register_data = register_data |+| 1;
    //     }
    // };

    // RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count7)
    // count_packet7 = {
    //     void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
    //         register_data = register_data |+| 1;
    //     }
    // };
    

    // RegisterAction<HASH_TABLE_KEY_BIT_WIDTH_t, bit<32>, HASH_TABLE_KEY_BIT_WIDTH_t>(hash_tbl_key8)
    // check_key8 = {
    //     void apply(inout HASH_TABLE_KEY_BIT_WIDTH_t register_data, out CHECK_FLAG result) {
    //         if(register_data != 0){
    //             if(register_data == meta.flow_id){
    //                 result = 1;
    //             }else{
    //                 result = 0;
    //             }
    //         }else{
    //             result = 1;
    //             register_data = meta.flow_id;
    //         }
    //         // register_data = register_data |+| 1;
    //     }
    // };

    // RegisterAction<HASH_TABLE_COUNT_BIT_WIDTH_t, bit<32>, HASH_TABLE_COUNT_BIT_WIDTH_t>(hash_tbl_count8)
    // count_packet8 = {
    //     void apply(inout HASH_TABLE_COUNT_BIT_WIDTH_t register_data) {
    //         register_data = register_data |+| 1;
    //     }
    // };
    


    
    cm_sketch_ingress() small;
    apply{
        if(meta.big_flow_flag==1){
            // big.apply(meta);
            if(check_key1.execute(meta.index_sketch1)==1){
                count_packet1.execute(meta.index_sketch1);
            }else if(check_key2.execute(meta.index_sketch1)==1){
                count_packet2.execute(meta.index_sketch1);
            }else if(check_key3.execute(meta.index_sketch1)==1){
                count_packet3.execute(meta.index_sketch1);
            }else if(check_key4.execute(meta.index_sketch1)==1){
                count_packet4.execute(meta.index_sketch1);
            }else{
                small.apply(meta);
            }
        }else{
            small.apply(meta);
        }
        // if(check_key1.execute(meta.index_sketch1)==1){
        //     count_packet1.execute(meta.index_sketch1);
        // }else if(check_key2.execute(meta.index_sketch1)==1){
        //     count_packet2.execute(meta.index_sketch1);
        // }else if(check_key3.execute(meta.index_sketch1)==1){
        //     count_packet3.execute(meta.index_sketch1);
        // }else if(check_key4.execute(meta.index_sketch1)==1){
        //     count_packet4.execute(meta.index_sketch1);
        // }else if(check_key5.execute(meta.index_sketch1)==1){
        //     count_packet5.execute(meta.index_sketch1);
        // }else if(check_key6.execute(meta.index_sketch1)==1){
        //     count_packet6.execute(meta.index_sketch1);
        // }else if(check_key7.execute(meta.index_sketch1)==1){
        //     count_packet7.execute(meta.index_sketch1);
        // }else if(check_key8.execute(meta.index_sketch1)==1){
        //     count_packet8.execute(meta.index_sketch1);
        // }else{
        //     small.apply(meta);
        // }
        // check_key1.execute(meta.index_sketch1);        
    }
    

    
}


