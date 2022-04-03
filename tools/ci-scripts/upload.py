# Post build script
import os
import sys
import json
import boto3
import glob
from github import Github

def main():
    print("Starting upload..")
    bucket_name = os.environ['UPLOAD_BUCKET']
    upload_prefix = os.environ['UPLOAD_PREFIX']
    release_number = os.environ['RELEASE_NUMBER']
    full_prefix = upload_prefix + release_number
    S3 = boto3.client('s3', region_name=os.environ['UPLOAD_REGION'], endpoint_url=os.environ['UPLOAD_ENDPOINT'])
    path = os.path.join(os.getcwd(), os.environ['ARTIFACT_PATTERN'])
    print("Checking for files to upload in {}..".format(path))
    files = glob.glob(path, recursive=False)
    for archiveFile in files:
        filePath, fileName = os.path.split(archiveFile)
        print("Uploading {}/{}..".format(full_prefix, fileName))
        S3.upload_file(os.path.join(filePath, fileName), bucket_name, full_prefix + '/' + fileName)
        S3.put_object_acl(ACL='public-read', Bucket=bucket_name, Key=full_prefix + '/' + fileName)
        print("Uploaded Artifact to S3: https://public.overte.org/{}/{}".format(full_prefix, fileName))
    print("Finished")

main()
