#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5; //Ограничение по кол-ву наиболее релевантных страниц

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) { //Разбиение строки на слова
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) { //Разбиение строки стоп-слов на вектор
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) { //Добавление документа и вычисление TF
        const auto words = ParseQuery(document);
        document_count_ += 1;
        double a = 1.0;
        double TF = a/words.plus_words.size(); 
        for (const string& word : words.plus_words) {
            word_to_document_freqs_[word][document_id] += TF;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const { // Сортирует по убыванию релевантности и меняет размер вектора
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    
    int document_count_ = 0;

    map<string, map<int, double>> word_to_document_freqs_;

    set<string> stop_words_;
    
    struct Query {
    set<string> plus_words;
    set<string> minus_words;
    };
    
    bool IsStopWord(const string& word) const { //Проверка на стоп-слова
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const { //Разбивает строку на слова и отбрасывает стоп-слова
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const { //Заполняет минус- и плюс-слова
        Query query_words;
        for (string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                word = word.substr(1);
                query_words.minus_words.insert(word);
        } else{
            query_words.plus_words.insert(word);
        }       
    } return query_words;
    }
    
    double IDF(const string& word) const {
        return static_cast<double> (log(document_count_ * 1.0 / word_to_document_freqs_.at(word).size()));
    }
    

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double> document_to_relevance;
        for (const auto& word : query_words.plus_words) {
            if (word_to_document_freqs_.count(word) != 0) {
                const double idf = IDF(word);
                for (const auto& [id, TF] : word_to_document_freqs_.at(word)) {
                  document_to_relevance[id] += TF * idf;
                }
            }
        }
        for (const string& word : query_words.minus_words) {
            if (word_to_document_freqs_.count(word) != 0) {
              for (const auto& [id, value] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(value);
              }
            }
        }
        for (const auto& [key, value] : document_to_relevance) {
            matched_documents.push_back({key, value});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
