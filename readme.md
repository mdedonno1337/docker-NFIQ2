NFIQ2 on docker
===============

This docker image provide a compilation environement for the NIST Finger Image Quality 2 (NFIQ2) software developped by the NIST (for more information regarding the NFIQ2, see the [offical website](https://www.nist.gov/services-resources/software/development-nfiq-20) or the [github repo](https://github.com/usnistgov/NFIQ2)).

## Get the source
The first step is to download the [NFIQ2 source code](http://biometrics.nist.gov/cs_links/quality/NFIQ_2/NFIQ2.tgz), and coping it in the same folder as the `Dockerfile` file.

For the moment, this docker image seems not work with the github version. Will be fixed in the future.

## How to build
The building process is very simple. First, install [docker](https://get.docker.com/).
The second step is to run this docker image as follow:

    docker build -t nfiq2 .

This command will produce a docker image with the compiled binaris in the `/NFIQ2/NFIQ2/bin/NFIQ2` folder.

## How to use

Since the docker image is separate of the host system, to run the NFIQ2 script on data present on the host, we have to share a volume while starting the docker image:

    docker run -it -v <host folder>:/data nfiq bash

It also possible to run directly the `NFIQ2` script from the host without having to type the commands in the docker-image bash:
    
    mdedonno@dockerdev:/mnt/hgfs/D/Library/docker-NFIQ2/src$ docker run -it -v $(pwd):/data nfiq2 /NFIQ2/NFIQ2/bin/NFIQ2
    USAGE:
      NFIQ2 <runMode> [specific run mode arguments]
      <runMode>: run mode of NFIQ2 tool, possible values
                  SINGLE, BATCH
    
      run mode SINGLE:
      ----------------
      NFIQ2 SINGLE <fingerprintImage> <imageFormat> <outputFeatureData> <outputSpeed>
    
        <fingerprintImage>: path and filename to a fingerprint image
        <imageFormat>: one of following values describing the fingerprint image format
                  BMP, WSQ
        <outputFeatureData>: if to print computed quality feature values
                 true, false
        <outputSpeed>: if to print speed of quality feature computation
                 true, false
    
      run mode BATCH:
      ---------------
      NFIQ2 BATCH <fingerprintImageList> <imageFormat> <resultList> <outputFeatureData> <outputSpeed> [<speedResultList>]
    
        <fingerprintImageList>: path and filename to a list of fingerprint images
        <imageFormat>: one of following values describing the fingerprint image format of all images in the input list
                  BMP, WSQ
        <resultList>: path and filename of the CSV output file that will contain NFIQ2 values and (optional) feature values
        <outputFeatureData>: if to add computed quality feature values to the resulting CSV output file
                 true, false
        <outputSpeed>: if to compute the speed of NFIQ2 computation
                 true, false
        <speedResultList>: path and filename of another CSV output file that will contain the NFIQ2 speed values (optional argument, only applied if outputSpeed = true)

## Export the executable to an other docker image

To use only the compiled binaries into an other docker-image, it is possible to use a two-stage docker image (as this one), changing the `Running environnement` as needed, or export a tar version with only the compiled binaries:

    docker run -it -v $(pwd):/data nfiq2 tar -zcvf /data/NFIQ2-linux-x84_64.tgz /NFIQ2

The tar file can the be used with the following `Dockerfile` image:

    FROM centos
    ADD NFIQ2-linux-x84_64.tgz /
    ENV LD_LIBRARY_PATH=/NFIQ2/libOpenCV/lib:/NFIQ2/biomdi/common/lib:/NFIQ2/biomdi/fingerminutia/lib

## How to test

To run the complianceTestSet, get a shell inside the docker image:

    docker run -it nfiq2 bash

go to the complianceTestSet folder:
    
    cd NFIQ2/complianceTestSet/

and run the test as usual:

    ./run_nfiq2_complianceTest.csh

Note that the `run_nfiq2_complianceTest.csh` outputs an error even if the test is OK:

    mdedonno@dockerdev:/mnt/hgfs/D/Library/docker-NFIQ2/src$ docker run -it nfiq2 bash
    [root@3f21e99ced3d /]# cd NFIQ2/complianceTestSet/
    [root@3f21e99ced3d complianceTestSet]# ./run_nfiq2_complianceTest.csh
    Running compliance test for ../NFIQ2/bin/NFIQ2 ...
    NFIQ 2.0 COMPLIANCE TEST STARTS NOW
    NFIQ2: Compute quality score for fingerprint images in list fpImageList.txt
           Time needed for initialization of module: 1135.600 ms
           Running batch computation ...
           FVC2000_Db1_10_8.bmp: NFIQ2 score = 56
           FVC2000_Db1_100_1.bmp: NFIQ2 score = 3
           FVC2000_Db1_100_2.bmp: NFIQ2 score = 4
           ...
           FVC2002_Db1_89_1.bmp: NFIQ2 score = 88
           Batch computation done
    Files my_nfiq_numbers.txt and complianceTest_NFIQ2_scores.csv differ
    
    NFIQ COMPLIANCE TEST OF (../NFIQ2/bin/NFIQ2) COMPLETED

If the NFIQ2 scores are produced, the compilation is OK. The file produced here deffers of the test file because of the presence of multiples columns in the output file that are not present in the test file. 
