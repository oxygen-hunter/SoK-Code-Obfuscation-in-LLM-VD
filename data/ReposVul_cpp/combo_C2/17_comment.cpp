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

static int getLineToStop(const std::string &fulltext) {
	int lineNo = 1;
	bool inString = false;
	unsigned int i = 0;
	int __state = 0;
	while (__state != -1) {
		switch (__state) {
			case 0:
				if (i >= fulltext.length()) {
					__state = 12;
				} else {
					__state = 1;
				}
				break;
			case 1:
				if (fulltext[i] == '\n') {
					lineNo++;
					i++;
					__state = 0;
				} else {
					__state = 2;
				}
				break;
			case 2:
				if (inString && fulltext.compare(i, 2, "\\\"") == 0) {
					i += 2;
					__state = 0;
				} else {
					__state = 3;
				}
				break;
			case 3:
				if (fulltext[i] == '"') {
					inString = !inString;
					i++;
					__state = 0;
				} else {
					__state = 4;
				}
				break;
			case 4:
				if (!inString && fulltext.compare(i, 2, "//") == 0) {
					i += 2;
					__state = 5;
				} else {
					__state = 6;
				}
				break;
			case 5:
				if (fulltext[i] != '\n' && i < fulltext.length()) {
					i++;
				} else {
					lineNo++;
					__state = 0;
				}
				break;
			case 6:
				if (!inString && fulltext.compare(i, 2, "/*") == 0) {
					i += 2;
					__state = 7;
				} else {
					__state = 10;
				}
				break;
			case 7:
				if (i < fulltext.length()) {
					__state = 8;
				} else {
					__state = 0;
				}
				break;
			case 8:
				if (fulltext.compare(i, 2, "*/") != 0 && i < fulltext.length()) {
					if (fulltext[i] == '\n') {
						lineNo++;
					}
					i++;
				} else {
					__state = 0;
				}
				break;
			case 10:
				if (fulltext[i] == '{') {
					return lineNo;
				} else {
					i++;
					__state = 0;
				}
				break;
			case 12:
				return lineNo;
		}
	}
	return lineNo;
}

static std::string getComment(const std::string &fulltext, int line) {
	if (line < 1) return "";

	unsigned int start = 0;
	for (; start < fulltext.length(); ++start) {
		if (line <= 1) break;
		if (fulltext[start] == '\n') line--;
	}

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
		if (comment[startText] == '"') inString = !inString;
		if (!inString) {
			if (comment.compare(startText, 2, "//") == 0) break;
			if (comment[startText] == ';' && noOfSemicolon > 0) return "";
			if (comment[startText] == ';') noOfSemicolon++;
		}
	}

	if (startText + 2 > comment.length()) return "";

	std::string result = comment.substr(startText + 2);
	return result;
}

static std::string getDescription(const std::string &fulltext, int line) {
	if (line < 1) return "";

	unsigned int start = 0;
	for (; start < fulltext.length(); ++start) {
		if (line <= 1) break;
		if (fulltext[start] == '\n') line--;
	}

	if (fulltext.compare(start, 2, "//") != 0) return "";

	start = start + 2;

	while (fulltext[start] == ' ' || fulltext[start] == '\t') start++;
	std::string retString = "";

	while (fulltext[start] != '\n') {
		if (fulltext.compare(start, 2, "//") == 0) {
			retString += " ";
			start++;
		} else {
			retString += fulltext[start];
		}
		start++;
	}
	return retString;
}

static GroupInfo createGroup(std::string comment, int lineNo) {
	GroupInfo groupInfo;
	std::string finalGroupName;

	boost::regex regex("\\[(.*?)\\]");
	boost::match_results<std::string::const_iterator>  match;
	while (boost::regex_search(comment, match, regex)) {
		std::string groupName = match[1].str();
		if (finalGroupName.empty()) {
			finalGroupName = groupName;
		} else {
			finalGroupName = finalGroupName + "-" + groupName;
		}
		groupName.clear();
		comment = match.suffix();
	}

	groupInfo.commentString = finalGroupName;
	groupInfo.lineNo = lineNo;
	return groupInfo;
}

static GroupList collectGroups(const std::string &fulltext) {
	GroupList groupList;
	int lineNo = 1;
	bool inString = false;
	unsigned int i = 0;
	int __state = 0;
	while (__state != -1) {
		switch (__state) {
			case 0:
				if (i >= fulltext.length()) {
					__state = 16;
				} else {
					__state = 1;
				}
				break;
			case 1:
				if (fulltext[i] == '\n') {
					lineNo++;
					i++;
					__state = 0;
				} else {
					__state = 2;
				}
				break;
			case 2:
				if (inString && fulltext.compare(i, 2, "\\\"") == 0) {
					i += 2;
					__state = 0;
				} else {
					__state = 3;
				}
				break;
			case 3:
				if (fulltext[i] == '"') {
					inString = !inString;
					i++;
					__state = 0;
				} else {
					__state = 4;
				}
				break;
			case 4:
				if (!inString && fulltext.compare(i, 2, "//") == 0) {
					i += 2;
					__state = 5;
				} else {
					__state = 6;
				}
				break;
			case 5:
				if (fulltext[i] != '\n' && i < fulltext.length()) {
					i++;
				} else {
					lineNo++;
					__state = 0;
				}
				break;
			case 6:
				if (!inString && fulltext.compare(i, 2, "/*") == 0) {
					i++;
					__state = 7;
				} else {
					__state = 0;
				}
				break;
			case 7:
				if (i < fulltext.length()) {
					i++;
					__state = 8;
				} else {
					__state = 0;
				}
				break;
			case 8:
				bool isGroup = true;
				std::string comment;
				__state = 9;
				break;
			case 9:
				if (fulltext.compare(i, 2, "*/") != 0 && i < fulltext.length()) {
					if (fulltext[i] == '\n') {
						lineNo++;
						isGroup = false;
					}
					comment += fulltext[i];
					i++;
				} else {
					__state = 15;
				}
				break;
			case 15:
				if (isGroup) {
					groupList.push_back(createGroup(comment, lineNo));
				}
				__state = 0;
				break;
			case 16:
				return groupList;
		}
	}
	return groupList;
}

void CommentParser::collectParameters(const std::string& fulltext, FileModule *root_module) {
	static auto EmptyStringLiteral(std::make_shared<Literal>(Value(std::string(""))));

	GroupList groupList = collectGroups(fulltext);
	int parseTill = getLineToStop(fulltext);

	for (auto &assignment : root_module->scope.assignments) {
		if (!assignment->getExpr()->isLiteral()) continue;

		int firstLine = assignment->location().firstLine();
		if (firstLine >= parseTill || (
			assignment->location().fileName() != "" &&
			assignment->location().fileName() != root_module->getFilename() &&
			assignment->location().fileName() != root_module->getFullpath()
			)) {
			continue;
		}

		AnnotationList *annotationList = new AnnotationList();

		std::string comment = getComment(fulltext, firstLine);
		shared_ptr<Expression> params = CommentParser::parser(comment.c_str());
		if (!params) {
			params = EmptyStringLiteral;
		}

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