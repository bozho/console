$name = 'ConsoleZ'
$url = 'https://github.com/cbucher/console/releases/download/1.15.0/ConsoleZ.x86.1.15.0.15253.zip'
$url64 = 'https://github.com/cbucher/console/releases/download/1.15.0/ConsoleZ.x64.1.15.0.15253.zip'
$tools = Split-Path -parent $MyInvocation.MyCommand.Definition

Install-ChocolateyZipPackage $name $url $tools $url64