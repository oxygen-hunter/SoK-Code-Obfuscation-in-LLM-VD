#include "comment.h"
#include "expression.h"
#include "annotation.h"
#include <string>
#include <vector>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/regex.hpp>

struct GroupInfo {
	std::string commentString;
	int lineNo;
};

typedef std::vector <GroupInfo> GroupList;

static int getLineToStop(const std::string &fulltext, int idx = 0, int lineNo = 1, bool inString = false) {
	if (idx >= fulltext.length()) return lineNo;
	lineNo += fulltext[idx] == '\n';
	inString = (fulltext[idx] == '"') ? !inString : inString;
	return (!inString && fulltext.compare(idx, 2, "//") == 0) ? getLineToStop(fulltext, idx + 2, lineNo, inString)
		: (!inString && fulltext.compare(idx, 2, "/*") == 0) ? getLineToStop(fulltext, fulltext.find("*/", idx) + 2, lineNo, inString)
		: (fulltext[idx] == '{') ? lineNo : getLineToStop(fulltext, idx + 1, lineNo, inString);
}

static std::string getComment(const std::string &fulltext, int line) {
	if (line < 1) return "";
	unsigned int start = 0;
	for (; start < fulltext.length(); ++start) if (line <= 1) break; else if (fulltext[start] == '\n') line--;
	int end = start + 1;
	while (end < fulltext.size() && fulltext[end] != '\n') end++;
	std::string comment = fulltext.substr(start, end - start);
	unsigned int startText = 0;
	int noOfSemicolon = 0;
	bool inString = false;
	for (; startText < comment.length() - 1; ++startText) {
		if (inString && comment.compare(startText, 2, "\\\"") == 0) {
			startText++;
			continue;
		}
		inString = (comment[startText] == '"') ? !inString : inString;
		if (!inString) {
			if (comment.compare(startText, 2, "//") == 0) break;
			if (comment[startText] == ';' && noOfSemicolon > 0) return "";
			if (comment[startText] == ';') noOfSemicolon++;
		}
	}
	if (startText + 2 > comment.length()) return "";
	return comment.substr(startText + 2);
}

static std::string getDescription(const std::string &fulltext, int line) {
	if (line < 1) return "";
	unsigned int start = 0;
	for (; start < fulltext.length(); ++start) if (line <= 1) break; else if (fulltext[start] == '\n') line--;
	if (fulltext.compare(start, 2, "//") != 0) return "";
	start += 2;
	while (fulltext[start] == ' ' || fulltext[start] == '\t') start++;
	std::string retString = "";
	while (fulltext[start] != '\n') {
		retString += (fulltext.compare(start, 2, "//") == 0) ? ' ' : fulltext[start];
		start++;
	}
	return retString;
}

static GroupInfo createGroup(std::string comment, int lineNo) {
	GroupInfo groupInfo;
	std::string finalGroupName;
	boost::regex regex("\\[(.*?)\\]");
	boost::match_results<std::string::const_iterator> match;
	while (boost::regex_search(comment, match, regex)) {
		finalGroupName = finalGroupName.empty() ? match[1].str() : finalGroupName + "-" + match[1].str();
		comment = match.suffix();
	}
	groupInfo.commentString = finalGroupName;
	groupInfo.lineNo = lineNo;
	return groupInfo;
}

static GroupList collectGroups(const std::string &fulltext, int idx = 0, int lineNo = 1, bool inString = false, GroupList groupList = {}, std::string comment = "", bool isGroup = true) {
	if (idx >= fulltext.length()) return groupList;
	lineNo += fulltext[idx] == '\n';
	inString = (fulltext[idx] == '"') ? !inString : inString;
	if (!inString && fulltext.compare(idx, 2, "//") == 0) return collectGroups(fulltext, fulltext.find('\n', idx) + 1, lineNo + 1, inString, groupList);
	if (!inString && fulltext.compare(idx, 2, "/*") == 0) {
		idx += 2;
		lineNo += fulltext.find("*/", idx) - idx;
		if (isGroup) groupList.push_back(createGroup(fulltext.substr(idx, fulltext.find("*/", idx) - idx), lineNo));
		return collectGroups(fulltext, fulltext.find("*/", idx) + 2, lineNo, inString, groupList);
	}
	return collectGroups(fulltext, idx + 1, lineNo, inString, groupList);
}

void CommentParser::collectParameters(const std::string& fulltext, FileModule *root_module) {
	static auto EmptyStringLiteral(std::make_shared<Literal>(Value(std::string(""))));
	GroupList groupList = collectGroups(fulltext);
	int parseTill = getLineToStop(fulltext);
	for (auto &assignment : root_module->scope.assignments) {
		if (!assignment->getExpr()->isLiteral()) continue;
		int firstLine = assignment->location().firstLine();
		if (firstLine >= parseTill || (assignment->location().fileName() != "" &&
			assignment->location().fileName() != root_module->getFilename() &&
			assignment->location().fileName() != root_module->getFullpath())) {
			continue;
		}
		AnnotationList *annotationList = new AnnotationList();
		std::string comment = getComment(fulltext, firstLine);
		shared_ptr<Expression> params = CommentParser::parser(comment.c_str());
		if (!params) params = EmptyStringLiteral;
		annotationList->push_back(Annotation("Parameter", params));
		std::string descr = getDescription(fulltext, firstLine - 1);
		if (descr != "") {
			shared_ptr<Expression> expr(new Literal(Value(descr)));
			annotationList->push_back(Annotation("Description", expr));
		}
		for (const auto &groupInfo : boost::adaptors::reverse(groupList)) {
			if (groupInfo.lineNo < firstLine) {
				shared_ptr<Expression> expr(new Literal(Value(groupInfo.commentString)));
				annotationList->push_back(Annotation("Group", expr));
				break;
			}
		}
		assignment->addAnnotations(annotationList);
	}
}