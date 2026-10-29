## v 0.5

- Fix reading the FN answer: the answer code is masked with `0x7F`, the high bit
  is the warning flag. An FN answer with warnings was treated as an error.
- Fix a race in `fn_uart_trx()`: wait until the RX DMA is armed before sending
  a request, otherwise a fast answer is lost.
- Disable the expansion module detection while the app holds USART (pins 13/14),
  otherwise the port cannot be acquired and the app aborts.
- Keep the USB CLI console enabled: it is independent from USART (pins 13/14),
  so logs stay available via `log` while the app is running. Removed the legacy
  `cli_vcp_disable()` call and the log-level suppression during transactions.

## v 0.4

- API: 1.3.2 support 

## v 0.3

- API: 60.4 support (SDK 100)


## v 0.1

- First betta version