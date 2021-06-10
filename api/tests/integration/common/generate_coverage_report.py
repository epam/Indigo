import indigo_coverage

def generate_coverage_report(indigo, reportFile):
    indigoCoveredFunctionsList = []
    indigoNotCoveredFunctionsList = []
    # Check covered functions
    indigoFunctionsNumber = 0
    for key, value in indigo._indigoCoverageDict.items():
        indigoFunctionsNumber += 1
        if value:
            indigoCoveredFunctionsList.append((value, key))
        else:
            indigoNotCoveredFunctionsList.append(key)
    indigoCoveredFunctionsList.sort(reverse=True)
    indigoNotCoveredFunctionsList.sort()
    # Generate report for Indigo
    indigoCoveragePercent = 100.0 * float(len(indigoCoveredFunctionsList)) / float(indigoFunctionsNumber)
    reportTextStringList = list()
    reportTextStringList.append('Indigo functions coverage is about %s percents (%s functions of %s)' % (
        indigoCoveragePercent, len(indigoCoveredFunctionsList), indigoFunctionsNumber))
    reportTextStringList.append('Covered functions and number of calls:')
    maxLen = 0
    for callNumber, function in indigoCoveredFunctionsList:
        if len(function) > maxLen:
            maxLen = len(function)
    for callNumber, function in indigoCoveredFunctionsList:
        resultString = '\t%s%s: %s' % (function, ' ' * (maxLen - len(function) + 1), callNumber)
        reportTextStringList.append(resultString)
    reportTextStringList.append('Uncovered functions:')
    for function in indigoNotCoveredFunctionsList:
        reportTextStringList.append('\t%s' % function)
    # Generate report for IndigoObject
    indigoObjectCoveredFunctionsList = []
    indigoObjectNotCoveredFunctionsList = []
    indigoObjectFunctionsNumber = 0
    for key, value in indigo._indigoObjectCoverageDict.items():
        indigoObjectFunctionsNumber += 1
        if value:
            indigoObjectCoveredFunctionsList.append((value, key))
        else:
            indigoObjectNotCoveredFunctionsList.append(key)
    # Sort coverage data
    indigoObjectCoveredFunctionsByTypeDict = {}
    for type in indigo._indigoObjectCoverageByTypeDict:
        indigoObjectCoveredFunctionsByTypeDict[type] = []
        for key, value in indigo._indigoObjectCoverageByTypeDict[type].items():
            if value:
                indigoObjectCoveredFunctionsByTypeDict[type].append((value, key))
    indigoObjectCoveredFunctionsList.sort(reverse=True)
    indigoObjectNotCoveredFunctionsList.sort()
    if indigoObjectFunctionsNumber:
        indigoObjectCoveragePercent = 100.0 * float(len(indigoObjectCoveredFunctionsList)) / float(indigoObjectFunctionsNumber)
    else: 
        indigoObjectCoveragePercent = 0.0
    reportTextStringList.append('\nIndigoObject functions coverage is about %s percents (%s functions of %s)' % (indigoObjectCoveragePercent, len(indigoObjectCoveredFunctionsList), indigoObjectFunctionsNumber))
    typeList = sorted(indigoObjectCoveredFunctionsByTypeDict.keys())
    for type in sorted(indigoObjectCoveredFunctionsByTypeDict.keys()):
        reportTextStringList.append('%s (#%s) functions coverage:' % (indigo_coverage.IndigoObjectTypeDict[type], type))
        maxLen = 0
        indigoObjectCoveredFunctionsByTypeDict[type].sort(reverse=True)
        for callNumber, function in indigoObjectCoveredFunctionsByTypeDict[type]:
            if len(function) > maxLen:
                maxLen = len(function)
        for callNumber, function in indigoObjectCoveredFunctionsByTypeDict[type]:
            resultString = '\t%s%s: %s' % (function, ' ' * (maxLen - len(function) + 1), callNumber)
            reportTextStringList.append(resultString)
    reportTextStringList.append('Uncovered functions:')
    for function in indigoObjectNotCoveredFunctionsList:
        reportTextStringList.append('\t%s' % (function))
    reportTextStringList.append('Used IndigoObject types: %s' % (typeList))
    # Write full report
    f =  open(reportFile, 'wt')
    f.write('\n'.join(reportTextStringList))
    f.close()