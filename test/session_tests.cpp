#include <gtest/gtest.h>

#include <string>

#include <cpr.h>

#include "server.h"


static Server* server = new Server();
auto base = server->GetBaseUrl();

TEST(RedirectTests, TemporaryDefaultRedirectTest) {
    auto url = Url{base + "/temporary_redirect.html"};
    Session session;
    session.SetUrl(url);
    auto response = session.Get();
    auto expected_text = std::string{"Hello world!"};
    EXPECT_EQ(expected_text, response.text);
    EXPECT_EQ(Url{base + "/hello.html"}, response.url);
    EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
    EXPECT_EQ(200, response.status_code);
}

TEST(RedirectTests, NoTemporaryRedirectTest) {
    auto url = Url{base + "/temporary_redirect.html"};
    Session session;
    session.SetUrl(url);
    session.SetRedirect(false);
    auto response = session.Get();
    auto expected_text = std::string{"Found"};
    EXPECT_EQ(expected_text, response.text);
    EXPECT_EQ(url, response.url);
    EXPECT_EQ(std::string{}, response.header["content-type"]);
    EXPECT_EQ(302, response.status_code);
}

TEST(RedirectTests, PermanentDefaultRedirectTest) {
    auto url = Url{base + "/permanent_redirect.html"};
    Session session;
    session.SetUrl(url);
    auto response = session.Get();
    auto expected_text = std::string{"Hello world!"};
    EXPECT_EQ(expected_text, response.text);
    EXPECT_EQ(Url{base + "/hello.html"}, response.url);
    EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
    EXPECT_EQ(200, response.status_code);
}

TEST(RedirectTests, NoPermanentRedirectTest) {
    auto url = Url{base + "/permanent_redirect.html"};
    Session session;
    session.SetUrl(url);
    session.SetRedirect(false);
    auto response = session.Get();
    auto expected_text = std::string{"Moved Permanently"};
    EXPECT_EQ(expected_text, response.text);
    EXPECT_EQ(url, response.url);
    EXPECT_EQ(std::string{}, response.header["content-type"]);
    EXPECT_EQ(301, response.status_code);
}

TEST(MaxRedirectsTests, ZeroMaxRedirectsSuccessTest) {
    auto url = Url{base + "/hello.html"};
    Session session;
    session.SetUrl(url);
    session.SetMaxRedirects(0);
    auto response = session.Get();
    auto expected_text = std::string{"Hello world!"};
    EXPECT_EQ(expected_text, response.text);
    EXPECT_EQ(url, response.url);
    EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
    EXPECT_EQ(200, response.status_code);
}

TEST(MaxRedirectsTests, ZeroMaxRedirectsFailureTest) {
    auto url = Url{base + "/permanent_redirect.html"};
    Session session;
    session.SetUrl(url);
    session.SetMaxRedirects(0);
    auto response = session.Get();
    EXPECT_EQ(std::string{}, response.text);
    EXPECT_EQ(url, response.url);
    EXPECT_EQ(std::string{}, response.header["content-type"]);
    EXPECT_EQ(301, response.status_code);
}

TEST(MaxRedirectsTests, OneMaxRedirectsSuccessTest) {
    auto url = Url{base + "/permanent_redirect.html"};
    Session session;
    session.SetUrl(url);
    session.SetMaxRedirects(1);
    auto response = session.Get();
    auto expected_text = std::string{"Hello world!"};
    EXPECT_EQ(expected_text, response.text);
    EXPECT_EQ(Url{base + "/hello.html"}, response.url);
    EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
    EXPECT_EQ(200, response.status_code);
}

TEST(MaxRedirectsTests, OneMaxRedirectsFailureTest) {
    auto url = Url{base + "/two_redirects.html"};
    Session session;
    session.SetUrl(url);
    session.SetMaxRedirects(1);
    auto response = session.Get();
    EXPECT_EQ(std::string{}, response.text);
    EXPECT_EQ(Url{base + "/permanent_redirect.html"}, response.url);
    EXPECT_EQ(std::string{}, response.header["content-type"]);
    EXPECT_EQ(301, response.status_code);
}

TEST(MaxRedirectsTests, TwoMaxRedirectsSuccessTest) {
    auto url = Url{base + "/two_redirects.html"};
    Session session;
    session.SetUrl(url);
    session.SetMaxRedirects(2);
    auto response = session.Get();
    auto expected_text = std::string{"Hello world!"};
    EXPECT_EQ(expected_text, response.text);
    EXPECT_EQ(Url{base + "/hello.html"}, response.url);
    EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
    EXPECT_EQ(200, response.status_code);
}

TEST(MultipleGetTests, BasicMultipleGetTest) {
    auto url = Url{base + "/hello.html"};
    Session session;
    session.SetUrl(url);
    for (int i = 0; i < 100; ++i) {
        auto response = session.Get();
        auto expected_text = std::string{"Hello world!"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
}

TEST(MultipleGetTests, UrlChangeMultipleGetTest) {
    Session session;
    {
        auto url = Url{base + "/hello.html"};
        session.SetUrl(url);
        auto response = session.Get();
        auto expected_text = std::string{"Hello world!"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
    {
        auto url = Url{base + "/basic.json"};
        session.SetUrl(url);
        auto response = session.Get();
        auto expected_text = std::string{"[\n"
                                         "  {\n"
                                         "    \"first_key\": \"first_value\",\n"
                                         "    \"second_key\": \"second_value\"\n"
                                         "  }\n"
                                         "]"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"application/octet-stream"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
}

TEST(MultipleGetTests, HeaderMultipleGetTest) {
    auto url = Url{base + "/header_reflect.html"};
    Session session;
    session.SetUrl(url);
    session.SetHeader(Header{{"hello", "world"}});
    for (int i = 0; i < 100; ++i) {
        auto response = session.Get();
        auto expected_text = std::string{"Header reflect"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(std::string{"world"}, response.header["hello"]);
        EXPECT_EQ(200, response.status_code);
    }
}

TEST(MultipleGetTests, HeaderChangeMultipleGetTest) {
    auto url = Url{base + "/header_reflect.html"};
    Session session;
    session.SetUrl(url);
    session.SetHeader(Header{{"hello", "world"}});
    {
        auto response = session.Get();
        auto expected_text = std::string{"Header reflect"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(std::string{"world"}, response.header["hello"]);
        EXPECT_EQ(200, response.status_code);
    }
    session.SetHeader(Header{{"key", "value"}});
    {
        auto response = session.Get();
        auto expected_text = std::string{"Header reflect"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(std::string{"value"}, response.header["key"]);
        EXPECT_EQ(200, response.status_code);
    }
}

TEST(MultipleGetTests, ParameterMultipleGetTest) {
    auto url = Url{base + "/hello.html"};
    Session session;
    session.SetUrl(url, Parameters{{"hello", "world"}});
    for (int i = 0; i < 100; ++i) {
        auto response = session.Get();
        auto expected_text = std::string{"Hello world!"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(Url{url + "?hello=world"}, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
}

TEST(MultipleGetTests, ParameterChangeMultipleGetTest) {
    auto url = Url{base + "/hello.html"};
    Session session;
    session.SetUrl(url, Parameters{{"hello", "world"}});
    {
        auto response = session.Get();
        auto expected_text = std::string{"Hello world!"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(Url{url + "?hello=world"}, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
    session.SetUrl(url, Parameters{{"key", "value"}});
    {
        auto response = session.Get();
        auto expected_text = std::string{"Hello world!"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(Url{url + "?key=value"}, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
}

TEST(MultipleGetTests, BasicAuthenticationMultipleGetTest) {
    auto url = Url{base + "/basic_auth.html"};
    Session session;
    session.SetUrl(url);
    session.SetAuth(Authentication{"user", "password"});
    for (int i = 0; i < 100; ++i) {
        auto response = session.Get();
        auto expected_text = std::string{"Header reflect"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
}

TEST(MultipleGetTests, BasicAuthenticationChangeMultipleGetTest) {
    auto url = Url{base + "/basic_auth.html"};
    Session session;
    session.SetUrl(url);
    session.SetAuth(Authentication{"user", "password"});
    {
        auto response = session.Get();
        auto expected_text = std::string{"Header reflect"};
        EXPECT_EQ(expected_text, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{"text/html"}, response.header["content-type"]);
        EXPECT_EQ(200, response.status_code);
    }
    session.SetAuth(Authentication{"user", "bad_password"});
    {
        auto response = session.Get();
        EXPECT_EQ(std::string{}, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{}, response.header["content-type"]);
        EXPECT_EQ(401, response.status_code);
    }
    session.SetAuth(Authentication{"bad_user", "password"});
    {
        auto response = session.Get();
        EXPECT_EQ(std::string{}, response.text);
        EXPECT_EQ(url, response.url);
        EXPECT_EQ(std::string{}, response.header["content-type"]);
        EXPECT_EQ(401, response.status_code);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(server);
    return RUN_ALL_TESTS();
}