/*
 * File:   newcunittest.c
 * Author: Adamansky Anton <anton@adamansky.com>
 *
 * Created on Oct 1, 2012, 3:13:44 PM
 */

#include "bson.h"


#include <stdio.h>
#include <stdlib.h>
#include "CUnit/Basic.h"
#include <assert.h>
#include "ejdb_private.h"
#include <locale.h>

/*
 * CUnit Test Suite
 */

static EJDB *jb;

int init_suite(void) {
    jb = ejdbnew();
    if (!ejdbopen(jb, "dbt2", JBOWRITER | JBOCREAT | JBOTRUNC)) {
        return 1;
    }
    return 0;
}

int clean_suite(void) {
    ejdbrmcoll(jb, "contacts", true);
    ejdbclose(jb);
    ejdbdel(jb);
    return 0;
}

void testAddData() {
    CU_ASSERT_PTR_NOT_NULL_FATAL(jb);
    bson_oid_t oid;
    EJCOLL *ccoll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL(ccoll);

    //Record 1
    bson a1;

    bson_init(&a1);
    bson_append_string(&a1, "name", "Антонов");
    bson_append_string(&a1, "phone", "333-222-333");
    bson_append_int(&a1, "age", 33);
    bson_append_long(&a1, "longscore", 0xFFFFFFFFFF01LL);
    bson_append_double(&a1, "dblscore", 0.333333);
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "city", "Novosibirsk");
    bson_append_string(&a1, "country", "Russian Federation");
    bson_append_string(&a1, "zip", "630090");
    bson_append_string(&a1, "street", "Pirogova");
    bson_append_int(&a1, "room", 334);
    bson_append_finish_object(&a1); //EOF address
    bson_append_start_array(&a1, "complexarr");
    bson_append_start_object(&a1, "0");
    bson_append_string(&a1, "foo", "bar");
    bson_append_string(&a1, "foo2", "bar2");
    bson_append_string(&a1, "foo3", "bar3");
    bson_append_finish_object(&a1);
    bson_append_start_object(&a1, "1");
    bson_append_string(&a1, "foo", "bar");
    bson_append_string(&a1, "foo2", "bar3");
    bson_append_finish_object(&a1);
    bson_append_int(&a1, "2", 333);
    bson_append_finish_array(&a1); //EOF complexarr
    CU_ASSERT_FALSE_FATAL(a1.err);
    bson_finish(&a1);
    ejdbsavebson(ccoll, &a1, &oid);
    bson_destroy(&a1);

    //Record 2
    bson_init(&a1);
    bson_append_string(&a1, "name", "Адаманский");
    bson_append_string(&a1, "phone", "444-123-333");
    bson_append_long(&a1, "longscore", 0xFFFFFFFFFF02LL);
    bson_append_double(&a1, "dblscore", 0.93);
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "city", "Novosibirsk");
    bson_append_string(&a1, "country", "Russian Federation");
    bson_append_string(&a1, "zip", "630090");
    bson_append_string(&a1, "street", "Pirogova");
    bson_append_finish_object(&a1);
    bson_append_start_array(&a1, "labels");
    bson_append_string(&a1, "0", "red");
    bson_append_string(&a1, "1", "green");
    bson_append_string(&a1, "2", "with gap, label");
    bson_append_finish_array(&a1);
    bson_append_start_array(&a1, "drinks");
    bson_append_int(&a1, "0", 4);
    bson_append_long(&a1, "1", 556667);
    bson_append_double(&a1, "2", 77676.22);
    bson_append_finish_array(&a1);

    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);
    ejdbsavebson(ccoll, &a1, &oid);
    bson_destroy(&a1);

    //Record 3
    bson_init(&a1);
    bson_append_string(&a1, "name", "Ivanov");
    bson_append_long(&a1, "longscore", 66);
    bson_append_double(&a1, "dblscore", 1.0);
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "city", "Petropavlovsk");
    bson_append_string(&a1, "country", "Russian Federation");
    bson_append_string(&a1, "zip", "683042");
    bson_append_string(&a1, "street", "Dalnaya");
    bson_append_finish_object(&a1);
    bson_append_start_array(&a1, "drinks");
    bson_append_int(&a1, "0", 41);
    bson_append_long(&a1, "1", 222334);
    bson_append_double(&a1, "2", 77676.22);
    bson_append_finish_array(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);

    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    bson_destroy(&a1);
}

void testSetIndex1() {
    EJCOLL *ccoll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(ccoll);
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "ab.c.d", JBIDXSTR));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "ab.c.d", JBIDXSTR | JBIDXNUM));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "ab.c.d", JBIDXDROPALL));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "address.zip", JBIDXSTR));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "name", JBIDXSTR));

    //Insert new record with active index
    //Record 4
    bson a1;
    bson_oid_t oid;
    bson_init(&a1);
    bson_append_string(&a1, "name", "John Travolta");
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "country", "USA");
    bson_append_string(&a1, "zip", "4499995");
    bson_append_finish_object(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);
    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    bson_destroy(&a1);


    //Update record 4 with active index
    //Record 4
    bson_init(&a1);
    bson_append_oid(&a1, "_id", &oid);
    bson_append_string(&a1, "name", "John Travolta2");
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "country", "USA");
    bson_append_string(&a1, "zip", "4499996");
    bson_append_finish_object(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);

    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    CU_ASSERT_TRUE(ejdbrmbson(ccoll, &oid));
    bson_destroy(&a1);

    //Save Travolta again
    bson_init(&a1);
    bson_append_oid(&a1, "_id", &oid);
    bson_append_string(&a1, "name", "John Travolta");
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "country", "USA");
    bson_append_string(&a1, "zip", "4499996");
    bson_append_string(&a1, "street", "Beverly Hills");
    bson_append_finish_object(&a1);
    bson_append_start_array(&a1, "labels");
    bson_append_string(&a1, "0", "yellow");
    bson_append_string(&a1, "1", "red");
    bson_append_string(&a1, "2", "black");
    bson_append_finish_array(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);
    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    bson_destroy(&a1);
}

void testQuery1() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    //for (int i = 0; i < TCLISTNUM(q1res); ++i) {
    //    void *bsdata = TCLISTVALPTR(q1res, i);
    //    bson_print_raw(stderr, bsdata, 0);
    //}
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.zip'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);


    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery2() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.zip'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery3() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.zip", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'sname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 20"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery4() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery5() {

    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "red");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery6() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "labels", JBIDXARR));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "red");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'alabels'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"red\" 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("4499996", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery7() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "with gap, label");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'alabels'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"with gap, label\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery8() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //"labels" : {"$in" : ["yellow", "green"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "labels");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "green");
    bson_append_string(&bsq1, "1", "yellow");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'alabels'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"green\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"yellow\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("yellow", TCLISTVALPTR(q1res, i), "labels.0"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.1"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //todo check hash tokens mode
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "labels", JBIDXDROPALL));

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "labels");
    bson_append_start_array(&bsq1, "$in");

    char nbuff[TCNUMBUFSIZ];
    for (int i = 0; i <= JBINOPTMAPTHRESHOLD; ++i) {
        bson_numstrn(nbuff, TCNUMBUFSIZ, i);
        if (i == 2) {
            bson_append_string(&bsq1, nbuff, "green");
        } else if (i == 8) {
            bson_append_string(&bsq1, nbuff, "yellow");
        } else {
            bson_append_string(&bsq1, nbuff, nbuff);
        }
    }
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "USING HASH TOKENS IN: labels"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("yellow", TCLISTVALPTR(q1res, i), "labels.0"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.1"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery9() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "labels", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "red");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("4499996", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }


    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery10() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.street", JBIDXSTR));

    //"address.street" : {"$in" : ["Pirogova", "Beverly Hills"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.street");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "Pirogova");
    bson_append_string(&bsq1, "1", "Beverly Hills");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.street'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 6"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Beverly Hills", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery11() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.street", JBIDXDROPALL));

    //"address.street" : {"$in" : ["Pirogova", "Beverly Hills"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.street");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "Pirogova");
    bson_append_string(&bsq1, "1", "Beverly Hills");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Beverly Hills", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery12() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //"labels" : {"$in" : ["yellow", "green"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "labels");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "green");
    bson_append_string(&bsq1, "1", "yellow");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("yellow", TCLISTVALPTR(q1res, i), "labels.0"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.1"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery13() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //"drinks" : {"$in" : [4, 77676.22]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "drinks");
    bson_append_start_array(&bsq1, "$in");
    bson_append_int(&bsq1, "0", 4);
    bson_append_double(&bsq1, "1", 77676.22);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));


    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_long(41, TCLISTVALPTR(q1res, i), "drinks.0"));
            CU_ASSERT_FALSE(bson_compare_long(222334, TCLISTVALPTR(q1res, i), "drinks.1"));
            CU_ASSERT_FALSE(bson_compare_double(77676.22, TCLISTVALPTR(q1res, i), "drinks.2"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_long(4, TCLISTVALPTR(q1res, i), "drinks.0"));
            CU_ASSERT_FALSE(bson_compare_long(556667, TCLISTVALPTR(q1res, i), "drinks.1"));
            CU_ASSERT_FALSE(bson_compare_double(77676.22, TCLISTVALPTR(q1res, i), "drinks.2"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery14() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "drinks", JBIDXARR));

    //"drinks" : {"$in" : [4, 77676.22]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "drinks");
    bson_append_start_array(&bsq1, "$in");
    bson_append_int(&bsq1, "0", 4);
    bson_append_double(&bsq1, "1", 77676.22);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'adrinks'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 21"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"4\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"77676.220000\" 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery15() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_double(&bsq1, "dblscore", 0.333333);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 8"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery16() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_double(&bsq1, "dblscore", 0.333333);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery17() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    //"dblscore" : {"$bt" : [0.95, 0.3]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_start_array(&bsq1, "$bt");
    bson_append_double(&bsq1, "0", 0.95);
    bson_append_double(&bsq1, "1", 0.333333);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);


    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 13"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    //Second query
    tcxstrclear(log);
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);

    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_start_array(&bsq1, "$bt");
    bson_append_double(&bsq1, "0", 0.95);
    bson_append_double(&bsq1, "1", 0.333333);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);


    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery18() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXARR));

    //{"name" : {$strand : ["Travolta", "John"]}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$strand");
    bson_append_string(&bsq1, "0", "Travolta");
    bson_append_string(&bsq1, "1", "John");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    /*bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on name
    bson_append_finish_object(&bshints);*/
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'aname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 4"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"John\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"Travolta\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    //Second query
    tcxstrclear(log);
    tclistdel(q1res);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXDROPALL));

    count = 0;
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);

    //Third query
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "labels", JBIDXARR));

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "labels");
    bson_append_start_array(&bsq1, "$strand");
    bson_append_string(&bsq1, "0", "red");
    bson_append_string(&bsq1, "1", "black");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    count = 0;
    tcxstrclear(log);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery19() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXARR));

    //{"name" : {$stror : ["Travolta", "Антонов", "John"]}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$stror");
    bson_append_string(&bsq1, "0", "Travolta");
    bson_append_string(&bsq1, "1", "Антонов");
    bson_append_string(&bsq1, "2", "John");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'aname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"John\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"Travolta\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"Антонов\" 1"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    //No-index query
    tcxstrclear(log);
    tclistdel(q1res);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXDROPALL));

    count = 0;
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery20() {
    //dblscore
    //{'dblscore' : {'$gte' : 0.93}}
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$gte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 10"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //GT

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$gt", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 9"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //NOINDEX
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    //NOINDEX GTE
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$gte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery21() {
    //{'dblscore' : {'lte' : 0.93}}
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    //LTE
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$lte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 12"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //LT
    //{'dblscore' : {'$lt' : 0.93}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$lt", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 11"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    //NOINDEX GTE
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$lte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery22() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.country", JBIDXSTR));

    //{"address.country" : {$begin : "Ru"}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.country");
    bson_append_string(&bsq1, "$begin", "Ru");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.country'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.country", JBIDXDROPALL));

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.country");
    bson_append_string(&bsq1, "$begin", "R");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery23() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_regex(&bsq1, "name", "(IvaNov$|АНтоноВ$)", "i");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery24() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_append_long(&bshints, "$skip", 1);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == TCLISTNUM(q1res) - 1) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_append_long(&bshints, "$skip", 1);
    bson_append_long(&bshints, "$max", 2);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: YES"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == TCLISTNUM(q1res) - 1) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //No order specified
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_long(&bshints, "$skip", 1);
    bson_append_long(&bshints, "$max", 2);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_long(&bshints, "$skip", 4);
    bson_append_long(&bshints, "$max", 2);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 4"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 0);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC
    bson_append_finish_object(&bshints);
    bson_append_long(&bshints, "$skip", 3);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == TCLISTNUM(q1res) - 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

}

void testQuery25() { //$or
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson obs[2];
    bson_init_as_query(&obs[0]);
    bson_append_string(&obs[0], "name", "Ivanov");
    bson_finish(&obs[0]);
    CU_ASSERT_FALSE_FATAL(obs[0].err);

    bson_init_as_query(&obs[1]);
    bson_append_string(&obs[1], "name", "Антонов");
    bson_finish(&obs[1]);
    CU_ASSERT_FALSE_FATAL(obs[1].err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, obs, 2, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(
                !bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name") ||
                !bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));

    }
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    for (int i = 0; i < 2; ++i) {
        bson_destroy(&obs[i]);
    }
}

void testQuery26() { //$not $nin
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //{'address.city' : {$not : 'Novosibirsk'}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.city");
    bson_append_string(&bsq1, "$not", "Novosibirsk");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(bson_compare_string("Novosibirsk", TCLISTVALPTR(q1res, i), "address.city"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Double negation {'address.city' : {$not : {'$not' : 'Novosibirsk'}}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.city");
    bson_append_start_object(&bsq1, "$not");
    bson_append_string(&bsq1, "$not", "Novosibirsk");
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(!bson_compare_string("Novosibirsk", TCLISTVALPTR(q1res, i), "address.city"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //"name" : {"$nin" : ["John Travolta", "Ivanov"]}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$nin");
    bson_append_string(&bsq1, "0", "John Travolta");
    bson_append_string(&bsq1, "1", "Ivanov");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        CU_ASSERT_TRUE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery27() { //$exists
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //{'address.room' : {$exists : true}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.room");
    bson_append_bool(&bsq1, "$exists", true);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //{'address.room' : {$exists : true}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.room");
    bson_append_bool(&bsq1, "$exists", false);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //{'address.room' : {$not :  {$exists : true}}} is equivalent to {'address.room' : {$exists : false}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.room");
    bson_append_start_object(&bsq1, "$not");
    bson_append_bool(&bsq1, "$exists", true);
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: NO"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testOIDSMatching() { //OID matching
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson_type bt;
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(contacts, q1, &count, 0, log);
    CU_ASSERT_TRUE(count > 0);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) { //first
        char soid[25];
        bson_oid_t *oid;
        void *bsdata = TCLISTVALPTR(q1res, i);
        bson_iterator it2;
        bt = bson_find_from_buffer(&it2, bsdata, JDBIDKEYNAME);
        CU_ASSERT_EQUAL_FATAL(bt, BSON_OID);
        oid = bson_iterator_oid(&it2);
        bson_oid_to_string(oid, soid);
        //fprintf(stderr, "\nOID: %s", soid);

        //OID in string form maching
        bson bsq2;
        bson_init_as_query(&bsq2);

        if (i % 2 == 0) {
            bson_append_string(&bsq2, JDBIDKEYNAME, soid);
        } else {
            bson_append_oid(&bsq2, JDBIDKEYNAME, oid);
        }

        bson_finish(&bsq2);
        CU_ASSERT_FALSE_FATAL(bsq2.err);

        TCXSTR *log2 = tcxstrnew();
        EJQ *q2 = ejdbcreatequery(jb, &bsq2, NULL, 0, NULL);
        TCLIST *q2res = ejdbqryexecute(contacts, q2, &count, 0, log2);
        CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log2), "PRIMARY KEY MATCHING:"));
        CU_ASSERT_EQUAL(count, 1);

        tcxstrdel(log2);
        ejdbquerydel(q2);
        tclistdel(q2res);
        bson_destroy(&bsq2);
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testEmptyFieldIndex() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXDROPALL));

    bson a1;
    bson_oid_t oid;
    bson_init(&a1);
    bson_append_string(&a1, "name", ""); //Empty but indexed field
    CU_ASSERT_FALSE_FATAL(a1.err);
    bson_finish(&a1);
    CU_ASSERT_TRUE(ejdbsavebson(coll, &a1, &oid));
    bson_destroy(&a1);
    CU_ASSERT_EQUAL(ejdbecode(coll->jb), 0);

    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXISTR)); //Ignore case string index
    CU_ASSERT_EQUAL(ejdbecode(coll->jb), 0);

    bson_init(&a1);
    bson_append_string(&a1, "name", ""); //Empty but indexed field
    CU_ASSERT_FALSE_FATAL(a1.err);
    bson_finish(&a1);
    CU_ASSERT_TRUE(ejdbsavebson(coll, &a1, &oid));
    bson_destroy(&a1);
    CU_ASSERT_EQUAL(ejdbecode(coll->jb), 0);
}

void testICaseIndex() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXISTR)); //Ignore case string index

    //Save one more record
    bson a1;
    bson_oid_t oid;
    bson_init(&a1);
    bson_append_string(&a1, "name", "HeLlo WorlD"); //#1
    CU_ASSERT_FALSE_FATAL(a1.err);
    bson_finish(&a1);
    CU_ASSERT_TRUE(ejdbsavebson(coll, &a1, &oid));
    bson_destroy(&a1);
    CU_ASSERT_EQUAL(ejdbecode(coll->jb), 0);

    bson_init(&a1);
    bson_append_string(&a1, "name", "THéÂtRE — театр"); //#2
    CU_ASSERT_FALSE_FATAL(a1.err);
    bson_finish(&a1);
    CU_ASSERT_TRUE(ejdbsavebson(coll, &a1, &oid));
    bson_destroy(&a1);
    CU_ASSERT_EQUAL(ejdbecode(coll->jb), 0);


    //Case insensitive query using index
    // {"name" : {"$icase" : "HellO woRLD"}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_string(&bsq1, "$icase", "HellO woRLD");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_TRUE(count == 1);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'iname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(!bson_compare_string("HeLlo WorlD", TCLISTVALPTR(q1res, i), "name"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //OK then drop icase index
    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXISTR | JBIDXDROP)); //Ignore case string index

    //Same query:
    //{"name" : {"$icase" : {$in : ["théâtre - театр", "hello world"]}}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_object(&bsq1, "$icase");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "théâtre - театр");
    bson_append_string(&bsq1, "1", "hello world");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_TRUE(count == 2);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(
                !bson_compare_string("HeLlo WorlD", TCLISTVALPTR(q1res, i), "name") ||
                !bson_compare_string("THéÂtRE — театр", TCLISTVALPTR(q1res, i), "name")
                );
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testTicket7() { //https://github.com/Softmotions/ejdb/issues/7
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);

    char xoid[25];
    bson_iterator it;
    bson_type bt;
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    const int onum = 3; //number of saved bsons
    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_TRUE_FATAL(count >= onum);

    for (int i = 0; i < TCLISTNUM(q1res) && i < onum; ++i) {
        void *bsdata = TCLISTVALPTR(q1res, i);
        CU_ASSERT_PTR_NOT_NULL_FATAL(bsdata);
    }
    //Now perform $in qry
    //{_id : {$in : ["oid1", "oid2", "oid3"]}}
    bson bsq2;
    bson_init_as_query(&bsq2);
    bson_append_start_object(&bsq2, "_id");
    bson_append_start_array(&bsq2, "$in");
    for (int i = 0; i < onum; ++i) {
        char ibuf[10];
        snprintf(ibuf, 10, "%d", i);
        bson_oid_t *oid = NULL;
        bt = bson_find_from_buffer(&it, TCLISTVALPTR(q1res, i), "_id");
        CU_ASSERT_TRUE_FATAL(bt == BSON_OID);
        oid = bson_iterator_oid(&it);
        CU_ASSERT_PTR_NOT_NULL_FATAL(oid);
        bson_oid_to_string(oid, xoid);
        //fprintf(stderr, "\ni=%s oid=%s", ibuf, xoid);
        if (i % 2 == 0) {
            bson_append_oid(&bsq2, ibuf, oid);
        } else {
            bson_append_string(&bsq2, ibuf, xoid);
        }
    }
    bson_append_finish_array(&bsq2);
    bson_append_finish_object(&bsq2);
    bson_finish(&bsq2);
    CU_ASSERT_FALSE_FATAL(bsq2.err);

    EJQ *q2 = ejdbcreatequery(jb, &bsq2, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q2);
    uint32_t count2 = 0;
    TCXSTR *log2 = tcxstrnew();
    TCLIST *q2res = ejdbqryexecute(coll, q2, &count2, 0, log2);
    //fprintf(stderr, "\n%s", TCXSTRPTR(log2));
    CU_ASSERT_TRUE(count2 == 3);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log2), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log2), "PRIMARY KEY MATCHING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log2), "RS COUNT: 3"));

    for (int i = 0; i < TCLISTNUM(q2res); ++i) {
        bson_oid_t *oid1 = NULL;
        bt = bson_find_from_buffer(&it, TCLISTVALPTR(q2res, i), "_id");
        CU_ASSERT_TRUE_FATAL(bt == BSON_OID);
        oid1 = bson_iterator_oid(&it);
        bool matched = false;
        for (int j = 0; j < TCLISTNUM(q1res); ++j) {
            bson_oid_t *oid2 = NULL;
            bt = bson_find_from_buffer(&it, TCLISTVALPTR(q1res, j), "_id");
            CU_ASSERT_TRUE_FATAL(bt == BSON_OID);
            oid2 = bson_iterator_oid(&it);
            if (!memcmp(oid1, oid2, sizeof (bson_oid_t))) {
                matched = true;
                void *ptr = tclistremove2(q1res, j);
                if (ptr) {
                    TCFREE(ptr);
                }
                break;
            }
        }
        CU_ASSERT_TRUE(matched);
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_destroy(&bsq2);
    tclistdel(q2res);
    tcxstrdel(log2);
    ejdbquerydel(q2);
}

void testTicket8() { //https://github.com/Softmotions/ejdb/issues/8
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshits1;
    bson_init_as_query(&bshits1);
    bson_append_start_object(&bshits1, "$fields");
    bson_append_int(&bshits1, "phone", 1);
    bson_append_int(&bshits1, "address.city", 1);
    bson_append_int(&bshits1, "labels", 1);
    bson_append_finish_object(&bshits1);
    bson_finish(&bshits1);
    CU_ASSERT_FALSE_FATAL(bshits1.err);


    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshits1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    //    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
    //        void *bsdata = TCLISTVALPTR(q1res, i);
    //        bson_print_raw(stderr, bsdata, 0);
    //    }

    bson_type bt;
    bson_iterator it;
    int ccount = 0;
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        void *bsdata = TCLISTVALPTR(q1res, i);
        CU_ASSERT_PTR_NOT_NULL_FATAL(bsdata);

        if (!bson_compare_string("333-222-333", TCLISTVALPTR(q1res, i), "phone")) {
            ++ccount;
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("_id", &it);
            CU_ASSERT_TRUE(bt == BSON_OID);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("address", &it);
            CU_ASSERT_TRUE(bt == BSON_OBJECT);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("address.city", &it);
            CU_ASSERT_TRUE(bt == BSON_STRING);
            CU_ASSERT_FALSE(strcmp("Novosibirsk", bson_iterator_string(&it)));
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("address.zip", &it);
            CU_ASSERT_TRUE(bt == BSON_EOO);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("age", &it);
            CU_ASSERT_TRUE(bt == BSON_EOO);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("name", &it);
            CU_ASSERT_TRUE(bt == BSON_EOO);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("labels", &it);
            CU_ASSERT_TRUE(bt == BSON_EOO);
        } else if (!bson_compare_string("444-123-333", TCLISTVALPTR(q1res, i), "phone")) {
            ++ccount;
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("_id", &it);
            CU_ASSERT_TRUE(bt == BSON_OID);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("address", &it);
            CU_ASSERT_TRUE(bt == BSON_OBJECT);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("address.city", &it);
            CU_ASSERT_TRUE(bt == BSON_STRING);
            CU_ASSERT_FALSE(strcmp("Novosibirsk", bson_iterator_string(&it)));
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("address.zip", &it);
            CU_ASSERT_TRUE(bt == BSON_EOO);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("age", &it);
            CU_ASSERT_TRUE(bt == BSON_EOO);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("name", &it);
            CU_ASSERT_TRUE(bt == BSON_EOO);
            bson_iterator_from_buffer(&it, bsdata);
            bt = bson_find_fieldpath_value("labels", &it);
            CU_ASSERT_TRUE(bt == BSON_ARRAY);
            CU_ASSERT_FALSE(bson_compare_string("red", bsdata, "labels.0"));
            CU_ASSERT_FALSE(bson_compare_string("green", bsdata, "labels.1"));
            CU_ASSERT_FALSE(bson_compare_string("with gap, label", bsdata, "labels.2"));
        }
    }
    CU_ASSERT_TRUE(ccount == 2);
    bson_destroy(&bsq1);
    bson_destroy(&bshits1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testUpdate1() { //https://github.com/Softmotions/ejdb/issues/9

    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);

    bson_iterator it;

    //q: {name : 'John Travolta', $set : {'labels' : ['black', 'blue'], 'age' : 58}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "John Travolta");
    bson_append_start_object(&bsq1, "$set");
    bson_append_start_array(&bsq1, "labels");
    bson_append_string(&bsq1, "0", "black");
    bson_append_string(&bsq1, "1", "blue");
    bson_append_finish_array(&bsq1);
    bson_append_int(&bsq1, "age", 58);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(1, count);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: YES"));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(!bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        bson_iterator_from_buffer(&it, TCLISTVALPTR(q1res, i));
        CU_ASSERT_TRUE(bson_find_from_buffer(&it, TCLISTVALPTR(q1res, i), "age") == BSON_EOO);
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //q2: {name : 'John Travolta', age: 58}
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "John Travolta");
    bson_append_int(&bsq1, "age", 58);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(1, TCLISTNUM(q1res));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: NO"));

    int age = 58;
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_long(age, TCLISTVALPTR(q1res, i), "age"));
        CU_ASSERT_FALSE(bson_compare_string("black", TCLISTVALPTR(q1res, i), "labels.0"));
        CU_ASSERT_FALSE(bson_compare_string("blue", TCLISTVALPTR(q1res, i), "labels.1"));
    }
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //q3: {name : 'John Travolta', '$inc' : {'age' : -1}}
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "John Travolta");
    bson_append_start_object(&bsq1, "$inc");
    bson_append_int(&bsq1, "age", -1);
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: YES"));
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //q4: {name : 'John Travolta', age: 57}
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "John Travolta");
    bson_append_int(&bsq1, "age", 57);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(1, TCLISTNUM(q1res));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: NO"));

    --age;
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_long(age, TCLISTVALPTR(q1res, i), "age"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testUpdate2() { //https://github.com/Softmotions/ejdb/issues/9
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "age", JBIDXNUM));

    //q: {name : 'John Travolta', '$inc' : {'age' : 1}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "John Travolta");
    bson_append_start_object(&bsq1, "$inc");
    bson_append_int(&bsq1, "age", 1);
    bson_append_finish_object(&bsq1);
    bson_append_start_object(&bsq1, "$set");
    bson_append_bool(&bsq1, "visited", true);
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "John Travolta");
    bson_append_int(&bsq1, "age", 58);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    count = 0;
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'nage'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 8"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_bool(true, TCLISTVALPTR(q1res, i), "visited"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

}

void testQueryBool() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_bool(&bsq1, "visited", true);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);


    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_EQUAL(count, 1);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    CU_ASSERT_TRUE(ejdbsetindex(coll, "visited", JBIDXNUM));

    bson_init_as_query(&bsq1);
    bson_append_bool(&bsq1, "visited", true);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'nvisited'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_EQUAL(count, 1);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testDropAll() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXSTR));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "HeLlo WorlD");
    bson_append_bool(&bsq1, "$dropall", true);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);


    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$DROPALL ON:"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'sname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Select again
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "HeLlo WorlD");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'sname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 0"));
    //fprintf(stderr, "\n\n%s", TCXSTRPTR(log));

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testTokens$begin() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXSTR));

    //q: {'name' : {'$begin' : ['Ада', 'John T']}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$begin");
    bson_append_string(&bsq1, "0", "Ада");
    bson_append_string(&bsq1, "1", "John T");
    bson_append_string(&bsq1, "2", "QWE J");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'sname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 22"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(!bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name") ||
                !bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //q: {'name' : {'$begin' : ['Ада', 'John T']}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$begin");
    bson_append_string(&bsq1, "0", "Ада");
    bson_append_string(&bsq1, "1", "John T");
    bson_append_string(&bsq1, "2", "QWE J");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);


    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXDROPALL));

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(!bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name") ||
                !bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testOneFieldManyConditions() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "age");
    bson_append_int(&bsq1, "$lt", 60);
    bson_append_int(&bsq1, "$gt", 50);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(!bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
    }
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void test$addToSet() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_append_start_object(&bsq1, "$addToSet");
    bson_append_string(&bsq1, "personal.tags", "tag1");
    bson_append_string(&bsq1, "labels", "green");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    ejdbqryexecute(coll, q1, &count, JBQRYCOUNT, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    bson_destroy(&bsq1);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //check updated  data
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);\
    CU_ASSERT_EQUAL(TCLISTNUM(q1res), 1);
    //fprintf(stderr, "\n\n%s", TCXSTRPTR(log));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_string("tag1", TCLISTVALPTR(q1res, i), "personal.tags.0"));
        CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.0"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Uppend more vals
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_append_start_object(&bsq1, "$addToSet");
    bson_append_string(&bsq1, "personal.tags", "tag2");
    bson_append_string(&bsq1, "labels", "green");
    //bson_append_int(&bsq1, "scores", 1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    count = 0;
    log = tcxstrnew();
    ejdbqryexecute(coll, q1, &count, JBQRYCOUNT, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    bson_destroy(&bsq1);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //check updated  data
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);\
    CU_ASSERT_EQUAL(TCLISTNUM(q1res), 1);
    //fprintf(stderr, "\n\n%s", TCXSTRPTR(log));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        //bson_print_raw(stderr, TCLISTVALPTR(q1res, i), 0);
        CU_ASSERT_FALSE(bson_compare_string("tag1", TCLISTVALPTR(q1res, i), "personal.tags.0"));
        CU_ASSERT_FALSE(bson_compare_string("tag2", TCLISTVALPTR(q1res, i), "personal.tags.1"));
        CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.0"));
        CU_ASSERT_FALSE(!bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.1"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Uppend more vals
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_append_start_object(&bsq1, "$inc");
    bson_append_int(&bsq1, "age", -1);
    bson_append_finish_object(&bsq1);
    bson_append_start_object(&bsq1, "$addToSet");
    bson_append_string(&bsq1, "personal.tags", "tag3");
    bson_append_string(&bsq1, "labels", "red");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    count = 0;
    log = tcxstrnew();
    ejdbqryexecute(coll, q1, &count, JBQRYCOUNT, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));
    bson_destroy(&bsq1);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    //check again
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);\
    CU_ASSERT_EQUAL(TCLISTNUM(q1res), 1);
    //fprintf(stderr, "\n\n%s", TCXSTRPTR(log));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        //bson_print_raw(stderr, TCLISTVALPTR(q1res, i), 0);
        CU_ASSERT_FALSE(bson_compare_string("tag1", TCLISTVALPTR(q1res, i), "personal.tags.0"));
        CU_ASSERT_FALSE(bson_compare_string("tag2", TCLISTVALPTR(q1res, i), "personal.tags.1"));
        CU_ASSERT_FALSE(bson_compare_string("tag3", TCLISTVALPTR(q1res, i), "personal.tags.2"));
        CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.0"));
        CU_ASSERT_FALSE(bson_compare_string("red", TCLISTVALPTR(q1res, i), "labels.1"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void test$pull() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_append_start_object(&bsq1, "$pull");
    bson_append_string(&bsq1, "personal.tags", "tag2");
    bson_append_string(&bsq1, "labels", "green");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    ejdbqryexecute(coll, q1, &count, JBQRYCOUNT, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "UPDATING MODE: YES"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    bson_destroy(&bsq1);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //check
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "name", "Антонов");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);\
    CU_ASSERT_EQUAL(TCLISTNUM(q1res), 1);
    //fprintf(stderr, "\n\n%s", TCXSTRPTR(log));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        //bson_print_raw(stderr, TCLISTVALPTR(q1res, i), 0);
        CU_ASSERT_FALSE(bson_compare_string("tag1", TCLISTVALPTR(q1res, i), "personal.tags.0"));
        CU_ASSERT_FALSE(bson_compare_string("tag3", TCLISTVALPTR(q1res, i), "personal.tags.1"));
        CU_ASSERT_FALSE(bson_compare_string("red", TCLISTVALPTR(q1res, i), "labels.0"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testFindInComplexArray() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "complexarr.foo", "bar");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_string("bar", TCLISTVALPTR(q1res, i), "complexarr.0.foo"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Check matching positional element
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "complexarr.1.foo", "bar");
    bson_finish(&bsq1);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "\n%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_string("bar", TCLISTVALPTR(q1res, i), "complexarr.1.foo"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Check simple el
    bson_init_as_query(&bsq1);
    bson_append_int(&bsq1, "complexarr.2", 333);
    bson_finish(&bsq1);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "\n%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_long(333, TCLISTVALPTR(q1res, i), "complexarr.2"));
    }
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Check simple el2
    bson_init_as_query(&bsq1);
    bson_append_int(&bsq1, "complexarr", 333);
    bson_finish(&bsq1);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "\n%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_long(333, TCLISTVALPTR(q1res, i), "complexarr.2"));
    }
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //$exists
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "complexarr.foo");
    bson_append_bool(&bsq1, "$exists", true);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    CU_ASSERT_EQUAL(count, 1);
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //$exists 2
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "complexarr.1");
    bson_append_bool(&bsq1, "$exists", true);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    CU_ASSERT_EQUAL(count, 1);
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //$exists 3
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "complexarr.4");
    bson_append_bool(&bsq1, "$exists", true);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    CU_ASSERT_EQUAL(count, 0);
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void test$elemMatch() {
    //{complexarr : {$elemMatch : {foo : 'bar', foo2 : 'bar2', foo3 : 'bar3'}}}
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "complexarr");
    bson_append_start_object(&bsq1, "$elemMatch");
    bson_append_string(&bsq1, "foo", "bar");
    bson_append_string(&bsq1, "foo2", "bar2");
    bson_append_string(&bsq1, "foo3", "bar3");
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    CU_ASSERT_EQUAL(count, 1);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_string("bar", TCLISTVALPTR(q1res, i), "complexarr.0.foo"));
        CU_ASSERT_FALSE(bson_compare_string("bar2", TCLISTVALPTR(q1res, i), "complexarr.0.foo2"));
        CU_ASSERT_FALSE(bson_compare_string("bar3", TCLISTVALPTR(q1res, i), "complexarr.0.foo3"));
    }
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "complexarr");
    bson_append_start_object(&bsq1, "$elemMatch");
    bson_append_string(&bsq1, "foo", "bar");
    bson_append_string(&bsq1, "foo2", "bar3");
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 0);
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "complexarr.foo", "bar");
    bson_append_string(&bsq1, "complexarr.foo2", "bar3");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    log = tcxstrnew();
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testTicket16() {
    EJCOLL *coll = ejdbcreatecoll(jb, "abcd", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    CU_ASSERT_EQUAL(coll->tdb->inum, 0);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "abcd", JBIDXISTR));
    CU_ASSERT_TRUE(ejdbsetindex(coll, "abcd", JBIDXNUM));
    CU_ASSERT_EQUAL(coll->tdb->inum, 2);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "abcd", JBIDXDROPALL));
    CU_ASSERT_EQUAL(coll->tdb->inum, 0);
}

void test$upsert() {
    EJCOLL *coll = ejdbcreatecoll(jb, "abcd", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "cde", "fgh"); //sel condition
    bson_append_start_object(&bsq1, "$upsert");
    bson_append_string(&bsq1, "cde", "fgh");
    bson_append_string(&bsq1, "ijk", "lmnp");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    CU_ASSERT_EQUAL(count, 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        bson_iterator it;
        CU_ASSERT_TRUE(bson_find_from_buffer(&it, TCLISTVALPTR(q1res, i), "_id") == BSON_OID);
        CU_ASSERT_FALSE(bson_compare_string("fgh", TCLISTVALPTR(q1res, i), "cde"));
        CU_ASSERT_FALSE(bson_compare_string("lmnp", TCLISTVALPTR(q1res, i), "ijk"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "cde", "fgh"); //sel condition
    bson_append_start_object(&bsq1, "$upsert");
    bson_append_string(&bsq1, "cde", "fgh");
    bson_append_string(&bsq1, "ijk", "lmnp+");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    log = tcxstrnew();
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "cde", "fgh");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    log = tcxstrnew();
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_FALSE(bson_compare_string("fgh", TCLISTVALPTR(q1res, i), "cde"));
        CU_ASSERT_FALSE(bson_compare_string("lmnp+", TCLISTVALPTR(q1res, i), "ijk"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testPrimitiveCases1() {
    EJCOLL *coll = ejdbcreatecoll(jb, "abcd", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    ejdbqryexecute(coll, q1, &count, JBQRYCOUNT, log);
    CU_ASSERT_EQUAL(count, 1);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SIMPLE COUNT(*): 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));

    bson_destroy(&bsq1);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //$dropall on whole collection
    bson_init_as_query(&bsq1);
    bson_append_bool(&bsq1, "$dropall", true);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    count = 0;
    log = tcxstrnew();
    ejdbqryexecute(coll, q1, &count, JBQRYCOUNT, log);
    CU_ASSERT_EQUAL(count, 1);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "VANISH WHOLE COLLECTION ON $dropall"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));

    bson_destroy(&bsq1);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testTicket29() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    CU_ASSERT_TRUE(ejdbsetindex(coll, "name", JBIDXARR));

    bson a1;
    bson_init(&a1);
    bson_append_string(&a1, "name", "Hello Мир");
    bson_append_long(&a1, "longscore", 77);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);

    bson_oid_t oid;
    CU_ASSERT_TRUE(ejdbsavebson(coll, &a1, &oid));
    bson_destroy(&a1);

    //{"name" : {$strand : ["Hello", "Мир"]}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$strand");
    bson_append_string(&bsq1, "0", "Hello");
    bson_append_string(&bsq1, "1", "Мир");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);
    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqryexecute(coll, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1res);
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"Hello\" 1"));

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testTicket28() {
    EJCOLL *coll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    //{$some:2}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_int(&bsq1, "$some", 2);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);
    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_EQUAL(ejdbecode(jb), JBEQERROR);
    CU_ASSERT_PTR_NULL(q1);
    bson_destroy(&bsq1);
}

int main() {

    setlocale(LC_ALL, "en_US.UTF-8");
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("t2", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "testAddData", testAddData)) ||
            (NULL == CU_add_test(pSuite, "testSetIndex1", testSetIndex1)) ||
            (NULL == CU_add_test(pSuite, "testQuery1", testQuery1)) ||
            (NULL == CU_add_test(pSuite, "testQuery2", testQuery2)) ||
            (NULL == CU_add_test(pSuite, "testQuery3", testQuery3)) ||
            (NULL == CU_add_test(pSuite, "testQuery4", testQuery4)) ||
            (NULL == CU_add_test(pSuite, "testQuery5", testQuery5)) ||
            (NULL == CU_add_test(pSuite, "testQuery6", testQuery6)) ||
            (NULL == CU_add_test(pSuite, "testQuery7", testQuery7)) ||
            (NULL == CU_add_test(pSuite, "testQuery8", testQuery8)) ||
            (NULL == CU_add_test(pSuite, "testQuery9", testQuery9)) ||
            (NULL == CU_add_test(pSuite, "testQuery10", testQuery10)) ||
            (NULL == CU_add_test(pSuite, "testQuery11", testQuery11)) ||
            (NULL == CU_add_test(pSuite, "testQuery12", testQuery12)) ||
            (NULL == CU_add_test(pSuite, "testQuery13", testQuery13)) ||
            (NULL == CU_add_test(pSuite, "testQuery14", testQuery14)) ||
            (NULL == CU_add_test(pSuite, "testQuery15", testQuery15)) ||
            (NULL == CU_add_test(pSuite, "testQuery16", testQuery16)) ||
            (NULL == CU_add_test(pSuite, "testQuery17", testQuery17)) ||
            (NULL == CU_add_test(pSuite, "testQuery18", testQuery18)) ||
            (NULL == CU_add_test(pSuite, "testQuery19", testQuery19)) ||
            (NULL == CU_add_test(pSuite, "testQuery20", testQuery20)) ||
            (NULL == CU_add_test(pSuite, "testQuery21", testQuery21)) ||
            (NULL == CU_add_test(pSuite, "testQuery22", testQuery22)) ||
            (NULL == CU_add_test(pSuite, "testQuery23", testQuery23)) ||
            (NULL == CU_add_test(pSuite, "testQuery24", testQuery24)) ||
            (NULL == CU_add_test(pSuite, "testQuery25", testQuery25)) ||
            (NULL == CU_add_test(pSuite, "testQuery26", testQuery26)) ||
            (NULL == CU_add_test(pSuite, "testQuery27", testQuery27)) ||
            (NULL == CU_add_test(pSuite, "testOIDSMatching", testOIDSMatching)) ||
            (NULL == CU_add_test(pSuite, "testEmptyFieldIndex", testEmptyFieldIndex)) ||
            (NULL == CU_add_test(pSuite, "testICaseIndex", testICaseIndex)) ||
            (NULL == CU_add_test(pSuite, "testTicket7", testTicket7)) ||
            (NULL == CU_add_test(pSuite, "testTicket8", testTicket8)) ||
            (NULL == CU_add_test(pSuite, "testUpdate1", testUpdate1)) ||
            (NULL == CU_add_test(pSuite, "testUpdate2", testUpdate2)) ||
            (NULL == CU_add_test(pSuite, "testQueryBool", testQueryBool)) ||
            (NULL == CU_add_test(pSuite, "testDropAll", testDropAll)) ||
            (NULL == CU_add_test(pSuite, "testTokens$begin", testTokens$begin)) ||
            (NULL == CU_add_test(pSuite, "testOneFieldManyConditions", testOneFieldManyConditions)) ||
            (NULL == CU_add_test(pSuite, "test$addToSet", test$addToSet)) ||
            (NULL == CU_add_test(pSuite, "test$pull", test$pull)) ||
            (NULL == CU_add_test(pSuite, "testFindInComplexArray", testFindInComplexArray)) ||
            (NULL == CU_add_test(pSuite, "test$elemMatch", test$elemMatch)) ||
            (NULL == CU_add_test(pSuite, "testTicket16", testTicket16)) ||
            (NULL == CU_add_test(pSuite, "test$upsert", test$upsert)) ||
            (NULL == CU_add_test(pSuite, "testPrimitiveCases1", testPrimitiveCases1)) ||
            (NULL == CU_add_test(pSuite, "testTicket29", testTicket29)) ||
            (NULL == CU_add_test(pSuite, "testTicket28", testTicket28))
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    int ret = CU_get_error() || CU_get_number_of_failures();
    CU_cleanup_registry();
    return ret;
}
