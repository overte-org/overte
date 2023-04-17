#!/bin/bash

projectPath=$(dirname $0)

helpFunction()
{
   echo ""
   echo "Usage: $0 -u <UnityPath> -p <ProjectPath>"
   echo -e "\t-u The path in which Unity exists"
   echo -e "\t-p The path to build the project files (Default: ${projectPath})"
   exit 1
}


while getopts "u:p" opt
do
   case "$opt" in
      u ) unityPath="$OPTARG" ;;
      p ) projectPath="$OPTARG" ;;
      ? ) helpFunction ;;
   esac
done

if [ -z "$unityPath" ]
then
   echo "Unity path was not provided";
   helpFunction
fi

${unityPath}/Unity -quit -batchmode -projectPath ${projectPath} -exportPackage "Assets" "avatarExporter.unitypackage"
