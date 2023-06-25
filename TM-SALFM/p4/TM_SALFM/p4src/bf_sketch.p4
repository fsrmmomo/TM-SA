control BF_Sketch(
    in my_egress_headers_t hdr,
    inout my_egress_metadata_t meta)
{
    CRCPolynomial<bit<32>>(
        coeff    = 0x04C11DB7,
        reversed = true,
        msb      = false,
        extended = false,
        init     = 0xFFFFFFFF,
        xor      = 0xFFFFFFFF) poly;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly) hash_algo1;

    CRCPolynomial<bit<32>>(
        coeff    = 0x741B8CD7,
        reversed = true,
        msb      = false,
        extended = false,
        init     = 0xFFFFFFFF,
        xor      = 0xFFFFFFFF) poly2;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly2) hash_algo2;

    CRCPolynomial<bit<32>>(
        coeff    = 0xDB710641,
        reversed = true,
        msb      = false,
        extended = false,
        init     = 0xFFFFFFFF,
        xor      = 0xFFFFFFFF) poly3;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, poly3) hash_algo3;

    action hash1(){
        meta.index1 = hash_algo1.get({ hdr.ipv4.src_addr });
    }

    action hash2(){
        meta.index2 = hash_algo2.get({ hdr.ipv4.src_addr });
    }

    action hash3(){
        meta.index3 = hash_algo3.get({ hdr.ipv4.src_addr });
    }

    table tbl_hash1{
        actions = {
            hash1;
        }
        const default_action = hash1(); 
        size = 1;
    }

    table tbl_hash2{
        actions = {
            hash2;
        }
        const default_action = hash2(); 
        size = 1;
    }

    table tbl_hash3{
        actions = {
            hash3;
        }
        const default_action = hash3(); 
        size = 1;
    }

    Random<bit<8>>() random;
    action get_random_num() {
        meta.random_num = random.get();
    }

    Register<bit<8>, bit<32>>(67584) count1;
    Register<bit<8>, bit<32>>(67584) count2;
    Register<bit<8>, bit<32>>(67584) count3;

    RegisterAction<bit<8>, bit<32>, bit<8>>(count1)
    leave_count1 = {
        void apply(inout bit<8> count, out bit<8> s) {
            count = count |+| 1;
            s = count;
        }
    };

    RegisterAction<bit<8>, bit<32>, bit<8>>(count2)
    leave_count2 = {
        void apply(inout bit<8> count, out bit<8> s) {
            count = count |+| 1;
            s = count;
        }
    };

    RegisterAction<bit<8>, bit<32>, bit<8>>(count3)
    leave_count3 = {
        void apply(inout bit<8> count, out bit<8> s) {
            count = count |+| 1;
            s = count;
        }
    };

    RegisterAction<bit<8>, bit<32>, bit<8>>(count3)
    pickup_count3 = {
        void apply(inout bit<8> count, out bit<8> s) {
            s = count;
        }
    };

    action get_index(){
        meta.index1 = meta.index1 & meta.mask;
        meta.index2 = meta.index2 & meta.mask;
        meta.index3 = meta.index3 & meta.mask;
        // meta.index4 = meta.index4 & meta.mask;
        // meta.index5 = meta.index5 & meta.mask;
        // meta.index6 = meta.index6 & meta.mask;
    }

    action send(){

    }

    action count1_add(){
        meta.s1 = leave_count1.execute(meta.index1);
        meta.s1 = meta.s1 >> 5;
    }

    action count2_add(){
        meta.s2 = leave_count2.execute(meta.index2);
        meta.s2 = meta.s1 >> 5;
    }

    action count3_add(){
        meta.s3 = leave_count3.execute(meta.index3);
        meta.s3 = meta.s1 >> 5;
    }

    table tbl_count1_operation {
        key = {
            meta.sketch_flag : exact;
        }
        actions = {
            count1_add;
            send;
        }
        const entries = {
            (0) : count1_add();
            (1) : send();
        }
        size = 2;
    }

    table tbl_count2_operation {
        key = {
            meta.sketch_flag : exact;
        }
        actions = {
            count2_add;
            send;
        }
        const entries = {
            (0) : count2_add();
            (1) : send();
        }
        size = 2;
    }

    table tbl_count3_operation {
        key = {
            meta.sketch_flag : exact;
        }
        actions = {
            count3_add;
            send;
        }
        const entries = {
            (0) : count3_add();
            (1) : send();
        }
        size = 2;
    }

    action update_insert1(in bit<8> random_num, out bool insert) {
        if (random_num & 0x01 == 0) {
            insert = true;
        } else {
            insert = false;
        }
    }
                
    apply{
        if(hdr.ipv4.isValid() && (hdr.tcp.isValid() || hdr.udp.isValid())){
            // Hash
            tbl_hash1.apply();
            tbl_hash2.apply();
            tbl_hash3.apply();

            get_index();

            get_random_num();
            if (meta.s1 == 0) {
                if (meta.random_num & 0x01 == 0) {
                    tbl_count1_operation.apply();
                }
            } else if (meta.s1 == 1) {
                if (meta.random_num & 0x03 == 0) {
                    tbl_count1_operation.apply();
                }
            } else if (meta.s1 == 2) {
                if (meta.random_num & 0x03 == 0) {
                    tbl_count1_operation.apply();
                }
            } else if (meta.s1 == 3) {
                if (meta.random_num & 0x07 == 0) {
                    tbl_count1_operation.apply();
                }
            } else if (meta.s1 == 4) {
                if (meta.random_num & 0x1f == 0) {
                    tbl_count1_operation.apply();
                }
            } else if (meta.s1 == 5) {
                if (meta.random_num & 0x3f == 0) {
                    tbl_count1_operation.apply();
                }
            } else if (meta.s1 == 6) {
                if (meta.random_num & 0x7f == 0) {
                    tbl_count1_operation.apply();
                }
            } else if (meta.s1 == 7) {
                if (meta.random_num & 0xff == 0) {
                    tbl_count1_operation.apply();
                }
            }

            get_random_num();
            if (meta.s2 == 0) {
                if (meta.random_num & 0x01 == 0) {
                    tbl_count2_operation.apply();
                }
            } else if (meta.s2 == 1) {
                if (meta.random_num & 0x03 == 0) {
                    tbl_count2_operation.apply();
                }
            } else if (meta.s2 == 2) {
                if (meta.random_num & 0x03 == 0) {
                    tbl_count2_operation.apply();
                }
            } else if (meta.s2 == 3) {
                if (meta.random_num & 0x07 == 0) {
                    tbl_count2_operation.apply();
                }
            } else if (meta.s2 == 4) {
                if (meta.random_num & 0x1f == 0) {
                    tbl_count2_operation.apply();
                }
            } else if (meta.s2 == 5) {
                if (meta.random_num & 0x3f == 0) {
                    tbl_count2_operation.apply();
                }
            } else if (meta.s2 == 6) {
                if (meta.random_num & 0x7f == 0) {
                    tbl_count2_operation.apply();
                }
            } else if (meta.s2 == 7) {
                if (meta.random_num & 0xff == 0) {
                    tbl_count2_operation.apply();
                }
            }


            get_random_num();
            if (meta.s3 == 0) {
                if (meta.random_num & 0x01 == 0) {
                    tbl_count3_operation.apply();
                }
            } else if (meta.s3 == 1) {
                if (meta.random_num & 0x03 == 0) {
                    tbl_count3_operation.apply();
                }
            } else if (meta.s3 == 2) {
                if (meta.random_num & 0x03 == 0) {
                    tbl_count3_operation.apply();
                }
            } else if (meta.s3 == 3) {
                if (meta.random_num & 0x07 == 0) {
                    tbl_count3_operation.apply();
                }
            } else if (meta.s3 == 4) {
                if (meta.random_num & 0x1f == 0) {
                    tbl_count3_operation.apply();
                }
            } else if (meta.s3 == 5) {
                if (meta.random_num & 0x3f == 0) {
                    tbl_count3_operation.apply();
                }
            } else if (meta.s3 == 6) {
                if (meta.random_num & 0x7f == 0) {
                    tbl_count3_operation.apply();
                }
            } else if (meta.s3 == 7) {
                if (meta.random_num & 0xff == 0) {
                    tbl_count3_operation.apply();
                }
            }

            
            // CM_Sketch
            // get_index();
            // get_inserts();
            // if (meta.insert1) {
            //     tbl_count1_operation.apply();
            // }
            // if (meta.insert2) {
            //     tbl_count2_operation.apply();
            // }
            // if (meta.insert3) {
            //     tbl_count3_operation.apply();
            // }
        }
    }

}