from url_shortener_client import OK, UrlShortenerClient


def show_step(title):
    print()
    print("=" * 72)
    print(title)
    print("=" * 72)


def require_ok(status, message):
    if status != OK:
        print(message)
        raise SystemExit(1)


def main():
    client = UrlShortenerClient("127.0.0.1", 9000)

    show_step("1. Cliente Python encurta uma URL via proxy TCP")
    status, short_url = client.encurta("https://www.ufsc.br")
    require_ok(status, "Erro ao encurtar URL")

    code = client.codigo_curto(short_url)
    print("URL curta recebida:", short_url)
    print("Codigo curto:", code)
    print("No proxy deve aparecer: [PROXY] encaminhando ENCURTA para REST")

    show_step("2. Primeira resolucao: deve gerar CACHE MISS")
    status, original_url = client.resolve(code)
    require_ok(status, "Erro ao resolver URL")

    print("URL original:", original_url)
    print("No proxy deve aparecer: [CACHE] MISS e depois [CACHE] STORE")

    show_step("3. Segunda resolucao do mesmo codigo: deve gerar CACHE HIT")
    status, original_url = client.resolve(code)
    require_ok(status, "Erro ao resolver URL pelo cache")

    print("URL original:", original_url)
    print("No proxy deve aparecer: [CACHE] HIT")
    print("No servidor REST nao deve aparecer novo [GET] para esse codigo")

    show_step("4. Acesso da URL curta pelo navegador")
    print("Abra esta URL no navegador antes de continuar:")
    print(short_url)
    print("No servidor REST deve aparecer: [REDIRECT]")
    input("Depois de testar no navegador, pressione Enter...")

    show_step("5. Remocao: deve invalidar o cache")
    status = client.remove_url(code)
    require_ok(status, "Erro ao remover URL")

    print("URL removida")
    print("No proxy deve aparecer: [CACHE] INVALIDATE")

    show_step("6. Resolver depois da remocao: deve retornar 404")
    status, _ = client.resolve(code)
    if status == OK:
        print("Erro: a URL removida ainda foi resolvida")
        raise SystemExit(1)

    print("Resultado esperado: codigo nao encontrado")
    print("No proxy deve aparecer: [CACHE] MISS e REST retornou 404")

    show_step("7. Capacidade do cache: 5 entradas")
    print("Agora o cliente vai criar 6 URLs e resolver cada uma uma vez.")
    print("Como o cache suporta apenas 5 entradas, a sexta resolucao força LRU.")

    lru_codes = []
    for index in range(1, 7):
        url = f"https://cache-demo-{index}.example.com"
        status, short_url = client.encurta(url)
        require_ok(status, f"Erro ao encurtar URL {index}")

        demo_code = client.codigo_curto(short_url)
        lru_codes.append(demo_code)

        status, resolved_url = client.resolve(demo_code)
        require_ok(status, f"Erro ao resolver URL {index}")

        print(f"{index}. {demo_code} -> {resolved_url}")

    print()
    print("No proxy deve aparecer:")
    print("[CACHE] STORE ... tamanho=5/5")
    print("[CACHE] FULL capacidade=5. A entrada menos recente sera removida")

    print()
    print("Agora resolvemos o primeiro codigo dessa sequencia novamente.")
    print("Como ele era o menos recente, deve ter saido do cache.")
    status, _ = client.resolve(lru_codes[0])
    require_ok(status, "Erro ao resolver primeiro codigo da sequencia LRU")
    print("No proxy deve aparecer [CACHE] MISS para esse primeiro codigo.")

    show_step("8. Limpeza das URLs usadas no teste de cache")
    for demo_code in lru_codes:
        response = client.comando_raw(f"REMOVE {demo_code}")
        print(f"REMOVE {demo_code}: {response}")

    show_step("9. Circuit Breaker")
    print("Para demonstrar o Circuit Breaker:")
    print("1. Deixe o proxy rodando.")
    print("2. Derrube apenas o servidor REST.")
    print(
        "   Sugestao: rode servidor e proxy separados com make run-server e make run-proxy."
    )
    print("3. Quando o servidor REST estiver parado, pressione Enter aqui.")
    input()

    for attempt in range(1, 6):
        response = client.comando_raw(f"ENCURTA https://falha-{attempt}.example.com")
        print(f"Tentativa {attempt}: {response}")

    print()
    print("No cliente, as primeiras respostas tendem a ser ERR 502 e depois ERR 503.")


if __name__ == "__main__":
    main()
