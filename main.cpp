#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <curl/curl.h>
#include<nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <vector>
#include <boost/locale.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace asio = boost::asio;            // from <boost/asio.hpp>
namespace fs = boost::filesystem;
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


struct DiagramData
{
    std::string diagramType;
    std::string prompt;
    std::string theme;
};

class FileWorker
{
public:
    std::vector<char> readFileBinary(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + path);
        }

        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> content(fileSize);
        file.read(content.data(), fileSize);
        file.close();
        return content;
    }

    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) {
            throw std::runtime_error("Could not open file: " + path);
        }
        return std::string((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
    }
};

class ResponseFiller
{
public:
    virtual bool fill(http::response<http::dynamic_body>* response, std::string& data) = 0;
};

class JsonFiller : public ResponseFiller
{
public:
    bool fill(http::response<http::dynamic_body>* response, std::string& data) override
    {
        try
        {
            response->set(http::field::content_type, "application/json");
            beast::ostream(response->body()) << data;
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
};

class FileFiller : public ResponseFiller
{
private:
    FileWorker fileWorker;
public:
    bool fill(http::response<http::dynamic_body>* response, std::string& data) override
    {
        try
        {
            response->set(http::field::content_type, "text/html");
            beast::ostream(response->body()) << fileWorker.readFile(data);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
};

class ContentDefiner
{
public:
    std::string getContentType(const std::string& path) {
        if (path.ends_with(".html")) return "text/html";
        if (path.ends_with(".css")) return "text/css";
        if (path.ends_with(".js")) return "application/javascript";
        if (path.ends_with(".png")) return "image/png";
        if (path.ends_with(".jpg") || path.ends_with(".jpeg")) return "image/jpeg";
        if (path.ends_with(".gif")) return "image/gif";
        if (path.ends_with(".svg")) return "image/svg+xml";
        return "application/octet-stream"; // Для других типов файлов
    }
};

class ImageFiller : public ResponseFiller
{
private:
    FileWorker fileWorker;
    ContentDefiner contentDefiner;
public:
    bool fill(http::response<http::dynamic_body>* response, std::string& data) override
    {

        try {
            if (contentDefiner.getContentType(data) == "image/png")
            {
                std::vector<char> file_content_binary = fileWorker.readFileBinary("." + data);
                response->result(http::status::ok);
                response->set(http::field::content_type, contentDefiner.getContentType(data));
                response->body().clear();
                beast::ostream(response->body()).write(file_content_binary.data(), file_content_binary.size());
                return true;
            }
            else {
                std::string file_content = fileWorker.readFile("." + data);
                response->result(http::status::ok);
                response->set(http::field::content_type, contentDefiner.getContentType(data));
                beast::ostream(response->body()) << file_content;
                return true;
            }

        }
        catch (const std::exception&) {
            response->result(http::status::not_found);
            response->set(http::field::content_type, "text/plain");
            beast::ostream(response->body()) << "File not found\r\n";
            return false;
        }
    }
};

class HttpClient {
public:
    HttpClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
    }

    ~HttpClient() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    std::string sendRequest(const std::string& url, const std::string& jsonData) {
        if (!curl) {
            throw std::runtime_error("Failed to initialize cURL");
        }

        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer <token for deepseek>");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            throw std::runtime_error("cURL error: " + std::string(curl_easy_strerror(res)));
        }

        return readBuffer;
    }

private:
    CURL* curl;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
        size_t newLength = size * nmemb;
        s->append((char*)contents, newLength);
        return newLength;
    }
};

class AiRequestBuilder {
public:
    std::string buildRequest(const std::string& prompt) {
        // Тело запроса
        std::string jsonData = R"({
            "model": "deepseek/deepseek-r1-distill-llama-70b:free",
            "messages": [
                {
                    "role": "user",
                    "content": "Write a DETAILED and COMPLEX MermaidJS declaration for")" + prompt + R"(".Use one of the following diagram types : classDiagram, flowchart, er, or sequence.Define stages as nodes with clear labels(e.g., A[Stage 1]).Use arrows(-->) or appropriate connectors for the chosen diagram type to connect stages in a logical sequence.Include at least 7 stages(e.g., Initial Registration, Document Verification, Approval, etc.).Each stage should have a clear purpose and flow.Add decision nodes(diamonds, e.g., C{Decision ? }) where the process branches(e.g., Approved ? with Yes and No paths).Ensure decision nodes have at least two outcomes.Break down complex stages into sub - steps using indentation or appropriate syntax for the chosen diagram type.Include at least one parallel process(e.g., Document Verification and Background Check happening simultaneously).Use subgraphs or equivalent grouping mechanisms to group related stages(e.g., subgraph Document Processing).Do not use custom styles(classDef or class).Do not use nested or invalid syntax(e.g., no && for parallel processes, no nested subgraphs).Add comments(%%) to explain complex parts of the diagram if necessary.Ensure the syntax is 100 % valid MermaidJS.Do not include invalid syntax or placeholders.Return ONLY the valid MermaidJS syntax.Do not include explanations, comments, or additional text.Example structure for flowchart : graph TD A[Stage 1] --> B[Stage 2] B--> C{Decision ? } C--> | Yes | D[Stage 3] C--> | No | E[Stage 4] subgraph Group D--> F[Sub - step 1] F--> G[Sub - step 2] end.Now, generate the MermaidJS code for ")" + prompt + R"(."
                }
            ]
        })";
        return jsonData;
    }
};

class CurlAi {
private:
    HttpClient httpClient;
    AiRequestBuilder requestBuilder;

public:
    CurlAi() = default;

    std::string sendRequest(const std::string& prompt) {
        std::string jsonRequest = requestBuilder.buildRequest(prompt);
        std::string jsonResponse = httpClient.sendRequest("https://openrouter.ai/api/v1/chat/completions", jsonRequest);
        return jsonResponse;
    }
};

class URLOven
{
protected:

    const std::string encodeURIComponent(std::string& str) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (char c : str) {
            if (isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~') {
                escaped << c;
                continue;
            }

            if (c == '$' || c == '&' || c == '+' || c == ',' || c == '/' || c == ':' || c == ';' || c == '=' || c == '?' || c == '@') {
                escaped << std::uppercase << '%' << std::setw(2) << (int)(unsigned char)c;
                continue;
            }

            if ((unsigned char)c >= 0x80) {
                escaped << std::uppercase << '%' << std::setw(2) << (int)(unsigned char)c;

            }
            else {
                escaped << std::uppercase << '%' << std::setw(2) << (int)(unsigned char)c;
            }
        }

        return escaped.str();
    }

    std::string extractMermaidDiagram(const std::string& input) {
        std::istringstream stream(input);
        std::string line;
        std::string result;
        bool isMermaidBlock = false;
        std::string mermaidStartMarker = "```mermaid";
        std::string mermaidEndMarker = "```";

        while (std::getline(stream, line)) {
            // Check if the line starts the MermaidJS block
            if (line.find(mermaidStartMarker) != std::string::npos) {
                isMermaidBlock = true;
                continue; // Skip the marker line itself
            }

            // Check if the line ends the MermaidJS block
            if (isMermaidBlock && line.find(mermaidEndMarker) != std::string::npos) {
                break; // Stop reading after the end marker
            }

            // If we're inside the MermaidJS block, append the line to the result
            if (isMermaidBlock) {
                result += line + "\n";
            }
        }

        return result;
    }

public:
    virtual ~URLOven() = default;

    virtual const std::string MakeUrl(DiagramData* data) = 0;

};

class URLHandMade : public URLOven
{
public:

    const std::string MakeUrl(DiagramData* data) override
    {
        json::object createDiagramUrlJson;

        createDiagramUrlJson["type"] = "mermaid";
        createDiagramUrlJson["data"] = data->diagramType + "\n " + data->prompt;

        std::string serializedJson = json::serialize(createDiagramUrlJson);

        const std::string diagramUrlStr = "create=" + encodeURIComponent(serializedJson);

        std::string link = "https://www.draw.io/?lightbox=1&" + data->theme + "=1&border=10&edit=_blank&" + diagramUrlStr; //wanted

        std::string encodedUrl = "https://www.draw.io/?lightbox=1&dark=1&border=10&" + data->diagramType + " " + data->prompt;
        json::object responseObj;

        responseObj["urlDrawio"] = link;

        std::string responseBody = json::serialize(responseObj);

        return responseBody;
    }
};

class URLAi : public URLOven
{
public:
    const std::string MakeUrl(DiagramData* data) override
    {
        CurlAi* curlToAi = new CurlAi();
        std::string jsonString = curlToAi->sendRequest(data->prompt);

        std::cout << "ДАННЫЕ ОТ ИИ - " << jsonString << '\n';

        nlohmann::json jsonObj = nlohmann::json::parse(jsonString);
        std::string mermaidDiagram = jsonObj["choices"][0]["message"]["content"];

        data->prompt = extractMermaidDiagram(mermaidDiagram);

        json::object createDiagramUrlJson;
        createDiagramUrlJson["type"] = "mermaid";
        createDiagramUrlJson["data"] = data->prompt;

        std::string serializedJson = json::serialize(createDiagramUrlJson);

        const std::string diagramUrlStr = "create=" + encodeURIComponent(serializedJson);

        std::string link = "https://www.draw.io/?lightbox=1&" + data->theme + "=1&border=10&edit=_blank&" + diagramUrlStr; //wanted

        json::object responseObj;

        responseObj["urlDrawio"] = link;

        std::string responseBody = json::serialize(responseObj);

        return responseBody;
    }
};

class ExtractData
{
public:
    DiagramData* getData(http::request<http::dynamic_body>* request)
    {
        DiagramData* data = new DiagramData();

        boost::beast::error_code ec;
        json::value valueFromClient = json::parse(beast::buffers_to_string(request->body().data()), ec);

        data->diagramType = valueFromClient.at("diagramType").as_string().c_str();
        data->prompt = valueFromClient.at("prompt").as_string().c_str();
        data->theme = valueFromClient.at("theme").as_string().c_str();

        return data;
    }
};

class RequestHandler
{
public:
    virtual ~RequestHandler() = default;
    virtual void handle(http::request<http::dynamic_body>* request, http::response<http::dynamic_body>* response) = 0;
};

class MainHandler : public RequestHandler {
public:
    void handle(http::request<http::dynamic_body>* request, http::response<http::dynamic_body>* response) override
    {
        std::unique_ptr<ResponseFiller> filler = std::make_unique<FileFiller>();
        std::string path = "index.html";
        filler->fill(response, path);
    }
};

enum class HandlerType
{
    URL_AI_HANDLER,
    URL_HAND_MADE_HANDLER
};

class UrlHandler : public RequestHandler
{
private:
    HandlerType type;
public:
    UrlHandler(HandlerType type) : type(type) {}

    void handle(http::request<http::dynamic_body>* request, http::response<http::dynamic_body>* response) override
    {
        ExtractData extractor;
        DiagramData* data = extractor.getData(request);

        URLOven* urlHandler;

        switch (type)
        {
        case HandlerType::URL_HAND_MADE_HANDLER:
            urlHandler = new URLHandMade();
            break;
        case HandlerType::URL_AI_HANDLER:
            urlHandler = new URLAi();
            break;
        default:
            throw std::invalid_argument("Invalid handler type");
            break;
        }

        std::string jsonStr = urlHandler->MakeUrl(data);

        delete urlHandler;

        ResponseFiller* filler = new JsonFiller();
        filler->fill(response, jsonStr);

        delete filler;
        delete data;
    }
};

class DefaultHandler : public RequestHandler
{
public:
    void handle(http::request<http::dynamic_body>* request, http::response<http::dynamic_body>* response) override
    {
        ResponseFiller* filler = new ImageFiller();
        std::string path = request->target();
        filler->fill(response, path);
    }
};

class Routing
{
private:
    ExtractData* extractor;
    http::request<http::dynamic_body>* request;
    http::response<http::dynamic_body>* response;
    DiagramData* data;
    std::unique_ptr<URLOven> url;
    std::unique_ptr<ResponseFiller> filler;
public:
    Routing(http::request<http::dynamic_body>* request, ExtractData* extractor, http::response<http::dynamic_body>* response) :
        request(request), extractor(extractor), response(response) {
    }
    ~Routing()
    {
        delete extractor;
        delete request;
    }

    void handleRoute()
    {
        std::string route = request->target();
        if (route == "/main")
        {
            filler = std::make_unique<FileFiller>();
            std::string path = "index.html";
            filler->fill(response, path);
        }
        else if (route == "/hand")
        {
            data = extractor->getData(request);

            url = std::make_unique<URLHandMade>();

            std::string jsonStr = url->MakeUrl(data);

            filler = std::make_unique<JsonFiller>();

            filler->fill(response, jsonStr);

        }
        else if (route == "/ai")
        {
            data = extractor->getData(request);

            url = std::make_unique<URLAi>();

            std::string jsonStr = url->MakeUrl(data);

            filler = std::make_unique<JsonFiller>();

            filler->fill(response, jsonStr);
        }
        else
        {
            filler = std::make_unique<ImageFiller>();
            std::string path = request->target();
            filler->fill(response, path);
        }
    }
};

class HandlerFactory {
public:
    RequestHandler* createHandler(const std::string& route) {
        if (route == "/main") {
            return new MainHandler();
        }
        else if (route == "/hand") {
            return new UrlHandler(HandlerType::URL_HAND_MADE_HANDLER);
        }
        else if (route == "/ai") {
            return new UrlHandler(HandlerType::URL_AI_HANDLER);
        }
        else {
            return new DefaultHandler();
        }
    }
};

class Router
{
private:
    HandlerFactory factory;
public:
    void handleRequest(http::request<http::dynamic_body>* request, http::response<http::dynamic_body>* response)
    {
        std::string route = request->target();

        RequestHandler* handler = factory.createHandler(route);
        handler->handle(request, response);
    }
};

class HTTPConnection : public std::enable_shared_from_this<HTTPConnection>
{
public:
    HTTPConnection(tcp::socket socket)
        : socket(std::move(socket))
    {
    }
    ~HTTPConnection()
    {
        delete routing;
    }

    // init the async operations associated with the connection
    void start()
    {
        readRequest();
        check_deadline();
    }

private:
    tcp::socket socket;

    // The buffer for performing reads.
    beast::flat_buffer buffer{ 8192 };

    http::request<http::dynamic_body> request;

    http::response<http::dynamic_body> response;

    // The timer for putting a deadline on connection processing.
    asio::steady_timer deadline{ socket.get_executor(), std::chrono::seconds(60) };

    Router* routing;

    // Asynchronously receive a complete request message.
    void readRequest()
    {
        std::shared_ptr<HTTPConnection> self = shared_from_this();

        http::async_read(
            socket,
            buffer,
            request,
            [self](beast::error_code ec,
                std::size_t bytes_transferred)
            {

                boost::ignore_unused(bytes_transferred);
                if (!ec)
                {
                    std::cout << "Client was connected" << '\n';
                    self->processRequest();
                }
                else
                {
                    std::cerr << "Connection error" << ec;
                }
            });
    }

    void processRequest()
    {
        response.version(request.version());
        response.keep_alive(false);
        beast::error_code ec;

        switch (request.method())
        {
        case http::verb::get:
            response.result(http::status::ok);
            response.set(http::field::server, "Beast");
            createResponse();
            break;
        case http::verb::post:
            response.result(http::status::ok);
            response.set(http::field::server, "Beast");
            createResponse();
            break;
        default:
            // We return responses indicating an error if
            // we do not recognize the request method.
            response.result(http::status::bad_request);
            response.set(http::field::content_type, "text/plain");
            beast::ostream(response.body())
                << "Invalid request-method '"
                << std::string(request.method_string())
                << "'";
            break;
        }

        write_response();
    }

    // Construct a response message based on the program state.
    void createResponse()
    {

        routing = new Router();

        routing->handleRequest(&request, &response);

    }

    // Asynchronously transmit the response message.
    void
        write_response()
    {
        auto self = shared_from_this();

        response.content_length(response.body().size());

        http::async_write(
            socket,
            response,
            [self](beast::error_code ec, std::size_t)
            {
                self->socket.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline.cancel();
            });
    }

    // Check whether we have spent enough time on this connection.
    void
        check_deadline()
    {
        auto self = shared_from_this();

        deadline.async_wait(
            [self](beast::error_code ec)
            {
                if (!ec)
                {
                    // Close socket to cancel any outstanding operation.
                    self->socket.close(ec);
                }
            });
    }
};

// so make a loop here to accept all connections to the server
void http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
    acceptor.async_accept(socket, [&](beast::error_code ec)
        {
            if (!ec)
            {
                std::make_shared<HTTPConnection>(std::move(socket))->start();
            }
            else
            {
                std::cerr << "http_server ACCEPT ERROR" << ec;
            }

            http_server(acceptor, socket);
        });
}

int main(int argc, char* argv[])
{
    try
    {

        unsigned short const port = argc > 1 ? static_cast<unsigned short>(std::atoi(argv[1])) : 8080;

        auto const address = asio::ip::make_address("127.0.0.1");

        asio::io_context io{ 1 };

        tcp::acceptor acceptor{ io, {address, port} };
        tcp::socket socket{ io };

        http_server(acceptor, socket);

        io.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << "1Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}